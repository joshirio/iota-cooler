#include "walletmanager.h"
#include "../utils/definitionholder.h"
#include "../utils/Qt-AES/qaesencryption.h"

#include <QtCore/QtGlobal>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QRandomGenerator>
#include <QtCore/QCryptographicHash>

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
}

bool WalletManager::createAndInitWallet(const QString &filePath)
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
                                                  QCryptographicHash::Sha1);

    QTextStream out(&destFile);
    out << m_magicString << "\n";
    out << "version:" << DefinitionHolder::WALLET_VERSION << "\n";
    out << "data_checksum:" << dataChecksumString << "\n";
    out << "aes_iv:" << aesIvVector.toBase64() << "\n";
    out << "aes_wallet_data:" << aesWalletData.toBase64();

    destFile.close();
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
    QByteArray data;

    //TODO convert internal json

    return data;
}

void WalletManager::decryptAndDeserializeWallet(const QByteArray &iv)
{
    //TODO convert data into internal json
}

WalletManager::WalletManager(QObject *parent) :
    QObject(parent), m_currentWalletOp(NoOp)
{
    m_magicString = "IOTACOOL";
}

WalletManager::~WalletManager()
{

}
