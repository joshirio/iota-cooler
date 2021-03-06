#include "walletmanager.h"
#include "../utils/definitionholder.h"
#include "../utils/Qt-AES/qaesencryption.h"

#include <QtCore/QtGlobal>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QRandomGenerator>
#include <QtCore/QCryptographicHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonParseError>
#include <QtCore/QStandardPaths>

WalletManager* WalletManager::m_instance = 0;

WalletManager& WalletManager::getInstance()
{
    if (!m_instance)
        m_instance = new WalletManager();
    return *m_instance;
}

void WalletManager::destroy()
{
    if (m_instance)
        delete m_instance;
    m_instance = 0;
}

QString WalletManager::getTmpMultisigSignFilePath()
{
    return QStandardPaths::standardLocations(QStandardPaths::TempLocation)
            .at(0) + "/iotacooler_wallet_multisig";
}

bool WalletManager::deleteTmpMultisifSignFile() {
    QString file = getTmpMultisigSignFilePath();
    if (QFile::exists(file)) {
        return QFile::remove(file);
    }
    return true;
}

void WalletManager::unlockWallet(const QString &encryptionKey)
{
    lockWallet();
    m_encryptionKey = encryptionKey;
}

void WalletManager::lockWallet()
{
    m_encryptionKey.clear();
    m_jsonObject = QJsonObject();
}

bool WalletManager::isLocked() const
{
    return m_encryptionKey.isEmpty();
}

QString WalletManager::getCurrentWalletPassphrase()
{
    checkLock();

    return m_encryptionKey;
}

bool WalletManager::createAndInitWallet(const QString &filePath, bool recoverFunds,
                                        WalletError &error)
{
    checkLock();

    //init JSON empty wallet
    QString defaultJson = "{\"onlineSeed\":\"\",\"currentColdWallet\":\"\",\"cleanColdWalletBackup"
                          "\":\"\",\"currentOperation\":1,\"currentOpArgs\":[],\"currentAddress"
                          "\":\"\",\"pastUsedAdresses\":[],\"pastSpendingTransactions"
                          "\":[],\"pastIncomingTransactions\":[]}";
    if (recoverFunds)
        defaultJson.replace("\"currentOpArgs\":[]", "\"currentOpArgs\":[\"recover\"]");

    m_jsonObject = QJsonDocument::fromJson(defaultJson.toUtf8()).object();
    return writeWalletToFile(filePath, error);
}

bool WalletManager::writeWalletToFile(const QString &filePath,
                                      WalletError &error)
{
    checkLock();

    QFile destFile(filePath);
    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        error.errorType = WalletError::WalletWriteError;
        error.errorString = tr("Failed to open file %1: %2")
                .arg(filePath).arg(destFile.errorString());
        return false;
    }

    QString dataChecksumString;
    QByteArray aesIvVector;
    QByteArray aesWalletData;

    aesIvVector = getRandomIv();
    aesWalletData = serializeAndEncryptWallet(aesIvVector);
    dataChecksumString = QCryptographicHash::hash(aesWalletData,
                                                  QCryptographicHash::Sha1).toHex();

    QTextStream out(&destFile);
    out << m_magicString << "\n";
    out << "version:" << DefinitionHolder::WALLET_VERSION << "\n";
    out << "data_checksum:" << dataChecksumString << "\n";
    out << "aes_iv:" << aesIvVector.toBase64() << "\n";
    out << "aes_wallet_data:" << aesWalletData.toBase64();

    destFile.close();
    return true;
}

bool WalletManager::readWalletFile(const QString &filePath,
                                   WalletError &error)
{
    checkLock();

    QFile srcFile(filePath);
    if (!srcFile.open(QIODevice::ReadOnly)) {
        error.errorType = WalletError::WalletReadError;
        error.errorString = tr("Failed to open file %1: %2")
                .arg(filePath).arg(srcFile.errorString());
        return false;
    }

    QString magic;
    QString dataChecksumString = "null";
    quint32 version = 0;
    QByteArray aesIvVector;
    QByteArray aesWalletData;
    QStringList tmp;
    QTextStream in(&srcFile);

    //magic number
    magic = in.readLine();
    if (magic != m_magicString) {
        error.errorType = WalletError::WalletReadError;
        error.errorString = tr("Invalid wallet file, magic number mismatch!");
        return false;
    }

    //wallet version
    bool k;
    tmp = in.readLine().split(":", QString::SkipEmptyParts);
    if (tmp.size() == 2)
        version = tmp.at(1).toUInt(&k);
    if ((!k) | (version > DefinitionHolder::WALLET_VERSION)) {
        error.errorType = WalletError::WalletReadError;
        error.errorString = tr("Unknown wallet version, please update "
                          "your software to a newer version and try again!");
        return false;
    }

    //wallet data checksum
    tmp = in.readLine().split(":", QString::SkipEmptyParts);
    if (tmp.size() == 2)
        dataChecksumString = tmp.at(1);

    //aes iv
    tmp = in.readLine().split(":", QString::SkipEmptyParts);
    if (tmp.size() == 2)
        aesIvVector = QByteArray::fromBase64(tmp.at(1).toLatin1());

    //aes wallet data
    tmp = in.readLine().split(":", QString::SkipEmptyParts);
    if (tmp.size() == 2)
        aesWalletData = QByteArray::fromBase64(tmp.at(1).toLatin1());

    //check checksum
    QString rc = QCryptographicHash::hash(aesWalletData,
                                          QCryptographicHash::Sha1).toHex();
    if (rc != dataChecksumString) {
        error.errorType = WalletError::WalletReadError;
        error.errorString = tr("Wallet checksum mismatch! "
                          "Possibly caused by wallet file corruption.");
        return false;
    }

    srcFile.close();
    return decryptAndDeserializeWallet(aesWalletData, aesIvVector, error);
}

WalletManager::WalletOp WalletManager::getCurrentWalletOp()
{
    checkLock();

    return (WalletOp) m_jsonObject.value("currentOperation").toInt();
}

void WalletManager::clearImportedMultisigFile()
{
    checkLock();

    m_jsonObject.insert("currentColdWallet", "");
}

void WalletManager::clearCleamBackupImportedMultisigFile()
{
    checkLock();

    m_jsonObject.insert("cleanColdWalletBackup", "");
}

bool WalletManager::saveWallet(const QString &walletFilePath,
                               bool importTmpMultisigFile,
                               bool importTmpCleanBackupMultisigFile,
                               WalletError &error)
{
    checkLock();

    bool ok = true;
    if (importTmpMultisigFile) {
        ok = importMultisigFile();
    }
    if (importTmpCleanBackupMultisigFile) {
        ok = ok && importMultisigFileAsCleanBackup();
    }
    if (ok) {
        ok = ok && writeWalletToFile(walletFilePath, error);
    }
    return ok;
}

bool WalletManager::restoreWallet(const QString &walletFilePath,
                                  bool exportTmpMultisigFile,
                                  bool exportTmpCleanBackupMultisigFile,
                                  WalletError &error)
{
    checkLock();

    bool ok = true;
    ok = readWalletFile(walletFilePath, error);
    if (ok) {
        if (exportTmpMultisigFile) {
            ok = ok && exportMultisigFile();
        }
        if (exportTmpCleanBackupMultisigFile) {
            ok = ok &&  exportCleanBackupMultisigFile();
        }
    }
    return ok;
}

void WalletManager::setCurrentWalletOp(WalletOp op,
                                       const QVariantList &opArgs)
{
    checkLock();

    int i = (int) op;
    m_jsonObject.insert("currentOperation", i);
    m_jsonObject.insert("currentOpArgs", QJsonArray::fromVariantList(opArgs));
}

WalletManager::WalletOp WalletManager::getCurrentWalletOp(QVariantList &opArgs)
{
    checkLock();

    int i = m_jsonObject.value("currentOperation").toInt();
    opArgs = m_jsonObject.value("currentOpArgs").toArray().toVariantList();
    WalletOp op = (WalletOp) i;
    return op;
}

QVariantList WalletManager::getPastUsedAddresses()
{
    checkLock();

    return m_jsonObject.value("pastUsedAdresses").toArray().toVariantList();
}

void WalletManager::addPastUsedAddress(const QString &address)
{
    checkLock();

    QJsonArray array = m_jsonObject.value("pastUsedAdresses").toArray();
    array.append(address);
    m_jsonObject.insert("pastUsedAdresses", array);
}

QString WalletManager::getCurrentAddress()
{
    checkLock();

    return m_jsonObject.value("currentAddress").toString();
}

void WalletManager::setCurrentAddress(const QString &address)
{
    checkLock();

    m_jsonObject.insert("currentAddress", address);
}

QString WalletManager::getOnlineSeed()
{
    checkLock();

    return m_jsonObject.value("onlineSeed").toString();
}

void WalletManager::setOnlineSeed(const QString &seed)
{
    checkLock();

    m_jsonObject.insert("onlineSeed", seed);
}

void WalletManager::addPastSpendingTx(const UtilsIOTA::Transation &transaction)
{
    checkLock();

    QJsonArray txListArray = m_jsonObject.value("pastSpendingTransactions").toArray();

    QJsonObject jsonTx;
    jsonTx.insert("tailTxHash", transaction.tailTxHash);
    jsonTx.insert("amount", transaction.amount);
    jsonTx.insert("spendingAddress", transaction.spendingAddress);
    jsonTx.insert("receivingAdress", transaction.receivingAddress);
    jsonTx.insert("tag", transaction.tag);
    jsonTx.insert("dateTime", transaction.dateTime.toString(Qt::ISODate));
    txListArray.append(jsonTx);

    m_jsonObject.insert("pastSpendingTransactions", txListArray);
}

void WalletManager::addPastIncomingTx(const UtilsIOTA::Transation &transaction)
{
    checkLock();
    QJsonArray txListArray = m_jsonObject.value("pastIncomingTransactions").toArray();

    QJsonObject jsonTx;
    jsonTx.insert("tailTxHash", transaction.tailTxHash);
    jsonTx.insert("amount", transaction.amount);
    jsonTx.insert("spendingAddress", transaction.spendingAddress);
    jsonTx.insert("receivingAdress", transaction.receivingAddress);
    jsonTx.insert("tag", transaction.tag);
    jsonTx.insert("dateTime", transaction.dateTime.toString(Qt::ISODate));
    txListArray.append(jsonTx);

    m_jsonObject.insert("pastIncomingTransactions", txListArray);
}

QList<UtilsIOTA::Transation> WalletManager::getPastSpendingTxs()
{
    checkLock();

    QList<UtilsIOTA::Transation> txList;
    QJsonArray txsJsonArray = m_jsonObject.value("pastSpendingTransactions").toArray();

    foreach (QJsonValue jTx, txsJsonArray) {
        QJsonObject jObj = jTx.toObject();
        UtilsIOTA::Transation tx;
        tx.tailTxHash = jObj.value("tailTxHash").toString();
        tx.amount = jObj.value("amount").toString();
        tx.spendingAddress = jObj.value("spendingAddress").toString();
        tx.receivingAddress = jObj.value("receivingAdress").toString();
        tx.tag = jObj.value("tag").toString();
        tx.dateTime = QDateTime::fromString(jObj.value("dateTime").toString(),
                                            Qt::ISODate);

        txList.append(tx);
    }

    return txList;
}

QList<UtilsIOTA::Transation> WalletManager::getPastIncomingTxs()
{
    checkLock();

    QList<UtilsIOTA::Transation> txList;
    QJsonArray txsJsonArray = m_jsonObject.value("pastIncomingTransactions").toArray();

    foreach (QJsonValue jTx, txsJsonArray) {
        QJsonObject jObj = jTx.toObject();
        UtilsIOTA::Transation tx;
        tx.tailTxHash = jObj.value("tailTxHash").toString();
        tx.amount = jObj.value("amount").toString();
        tx.spendingAddress = jObj.value("spendingAddress").toString();
        tx.receivingAddress = jObj.value("receivingAdress").toString();
        tx.tag = jObj.value("tag").toString();
        tx.dateTime = QDateTime::fromString(jObj.value("dateTime").toString(),
                                            Qt::ISODate);

        txList.append(tx);
    }

    return txList;
}

void WalletManager::backupMultisigFileAsClean()
{
    checkLock();

    m_jsonObject.insert("cleanColdWalletBackup",
                        m_jsonObject.value("currentColdWallet"));
}

void WalletManager::restoreCleanMultisigFileBackup()
{
    checkLock();

    m_jsonObject.insert("currentColdWallet",
                        m_jsonObject.value("cleanColdWalletBackup"));
}

QString WalletManager::getRawJsonData() const
{
    QString s;

    if (isLocked())
        s = "no wallet loaded";
    else
        s = QString::fromUtf8(QJsonDocument(m_jsonObject).toJson());

    return s;
}

bool WalletManager::importMultisigFile()
{
    checkLock();

    QFile tmpFile(getTmpMultisigSignFilePath());
    if (!tmpFile.open(QIODevice::ReadOnly))
        return false;

    //purge smidgen history (too much data)
    QJsonParseError jerr;
    QJsonObject smidgenJson = QJsonDocument::fromJson(tmpFile.readAll(), &jerr).object();
    if (jerr.error != QJsonParseError::NoError) return false;
    smidgenJson.remove("last");

    m_jsonObject.insert("currentColdWallet",
                        QString::fromUtf8(QJsonDocument(smidgenJson)
                                          .toJson().toBase64()));
    return true;
}

bool WalletManager::importMultisigFileAsCleanBackup()
{
    checkLock();

    QFile tmpFile(getTmpMultisigSignFilePath());
    if (!tmpFile.open(QIODevice::ReadOnly))
        return false;

    //purge smidgen history (too much data)
    QJsonParseError jerr;
    QJsonObject smidgenJson = QJsonDocument::fromJson(tmpFile.readAll(), &jerr).object();
    if (jerr.error != QJsonParseError::NoError) return false;
    smidgenJson.remove("last");

    m_jsonObject.insert("cleanColdWalletBackup",
                        QString::fromUtf8(QJsonDocument(smidgenJson)
                                          .toJson().toBase64()));
    return true;
}

bool WalletManager::exportMultisigFile()
{
    checkLock();

    QFile tmpFile(getTmpMultisigSignFilePath());
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    qint64 w = tmpFile.write(QByteArray::fromBase64(m_jsonObject
                                                .value("currentColdWallet")
                                                .toString().toUtf8()));
    bool writeOK = w > 0;
    return writeOK;
}

bool WalletManager::exportCleanBackupMultisigFile()
{
    checkLock();

    QFile tmpFile(getTmpMultisigSignFilePath());
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    qint64 w = tmpFile.write(QByteArray::fromBase64(m_jsonObject
                                                .value("cleanColdWalletBackup")
                                                .toString().toUtf8()));
    bool writeOK = w > 0;
    return writeOK;
}

void WalletManager::checkLock() const
{
    if (m_encryptionKey.isEmpty())
        qFatal("[FATAL ERROR:] encryption key not set, call WalletManager::unlockWallet() first!");
}

QByteArray WalletManager::getRandomIv()
{
    //generate 128 random bits for the IV
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);

    stream << QRandomGenerator::system()->generate64(); //64 bits
    stream << QRandomGenerator::system()->generate64(); //64 bits

    return byteArray;
}

QByteArray WalletManager::serializeAndEncryptWallet(const QByteArray &iv)
{
    checkLock();

    QByteArray data;

    //convert internal json object to text json
    QJsonDocument doc(m_jsonObject);
    data = doc.toJson(QJsonDocument::Compact);

    //encrypt data
    QAESEncryption encryption(QAESEncryption::AES_256,
                              QAESEncryption::CBC,
                              QAESEncryption::ZERO);
    QByteArray hashKey = QCryptographicHash::hash(m_encryptionKey.toLocal8Bit(),
                                                  QCryptographicHash::Sha256);

    return encryption.encode(data, hashKey, iv);
}

bool WalletManager::decryptAndDeserializeWallet(const QByteArray &aesData,
                                                const QByteArray &iv,
                                                WalletError &error)
{
    checkLock();

    //clear
    m_jsonObject = QJsonObject();

    //decrypt data
    QAESEncryption encryption(QAESEncryption::AES_256,
                              QAESEncryption::CBC,
                              QAESEncryption::ZERO);
    QByteArray hashKey = QCryptographicHash::hash(m_encryptionKey.toLocal8Bit(),
                                                  QCryptographicHash::Sha256);

    QByteArray rawJson = encryption.decode(aesData, hashKey, iv);
    rawJson = encryption.removePadding(rawJson);
    QString jsonString = QString::fromUtf8(rawJson);
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        if (parseError.error == QJsonParseError::IllegalValue) {
            error.errorType = WalletError::WalletInvalidPassphrase;
            error.errorString = tr("Wallet password not correct");
        } else {
            error.errorType = WalletError::WalletFileParsingError;
            error.errorString = parseError.errorString();
        }
        return false;
    } else {
        m_jsonObject = doc.object();
    }

    return true;
}

WalletManager::WalletManager(QObject *parent) :
    QObject(parent), m_currentWalletOp(NoOp)
{
    m_magicString = "IOTACOOL";
}

WalletManager::~WalletManager()
{
    //clean up tmp file
    deleteTmpMultisifSignFile();
}
