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

bool WalletManager::createAndInitWallet(const QString &filePath)
{
    checkLock();

    //init JSON empty wallet
    QString defaultJson = "{\"onlineSeed\":\"\",\"currentColdWallet\":\"\",\"cleanColdWalletBackup"
                          "\":\"\",\"currentOperation\":1,\"currentOpArgs\":[],\"currentAddress"
                          "\":\"\",\"pastUsedAdresses\":[],\"pastSpendingTransactions\":[]}";
    m_jsonObject = QJsonDocument::fromJson(defaultJson.toUtf8()).object();

    return writeWalletToFile(filePath);
}

bool WalletManager::writeWalletToFile(const QString &filePath)
{
    checkLock();

    QFile destFile(filePath);
    QString errorMessage;

    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorMessage = tr("Failed to open file %1: %2")
                .arg(filePath).arg(destFile.errorString());
        emit walletWriteError(errorMessage);
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

bool WalletManager::readWalletFile(const QString &filePath)
{
    checkLock();

    QFile srcFile(filePath);
    QString errorMessage;

    if (!srcFile.open(QIODevice::ReadOnly)) {
        errorMessage = tr("Failed to open file %1: %2")
                .arg(filePath).arg(srcFile.errorString());
        emit walletReadError(errorMessage);
        return false;
    }

    QString magic, dataChecksumString;
    quint32 version = 0;
    QByteArray aesIvVector;
    QByteArray aesWalletData;
    QStringList tmp;

    QTextStream in(&srcFile);
    magic = in.readLine();
    if (magic != m_magicString) {
        errorMessage = tr("Invalid wallet file, magic number mismatch!");
        emit walletReadError(errorMessage);
        return false;
    }
    bool k;
    tmp = in.readLine().split(":", QString::SkipEmptyParts);
    if (tmp.size() == 2)
        version = tmp.at(1).toUInt(&k);
    if ((!k) | (version > DefinitionHolder::WALLET_VERSION)) {
        errorMessage = tr("Unknown wallet version, please update "
                          "your software to a newer version and try again!");
        emit walletReadError(errorMessage);
        return false;
    }

    //TODO

    srcFile.close();
    return true;
}

void WalletManager::checkLock()
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
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray hashKey = QCryptographicHash::hash(m_encryptionKey.toLocal8Bit(),
                                                  QCryptographicHash::Sha256);

    return encryption.encode(data, hashKey, iv);
}

void WalletManager::decryptAndDeserializeWallet(const QByteArray &aesData,
                                                const QByteArray &iv)
{
    checkLock();

    //clear
    m_jsonObject = QJsonObject();

    //decrypt data
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray hashKey = QCryptographicHash::hash(m_encryptionKey.toLocal8Bit(),
                                                  QCryptographicHash::Sha256);

    QByteArray rawJson = encryption.decode(aesData, hashKey, iv);
    QString cleanJson(rawJson); //remove \0 to make a valid string
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(cleanJson.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit walletFileParsingError(parseError.errorString());
    } else {
        m_jsonObject = doc.object();
    }
}

WalletManager::WalletManager(QObject *parent) :
    QObject(parent), m_currentWalletOp(NoOp)
{
    m_magicString = "IOTACOOL";
}

WalletManager::~WalletManager()
{

}
