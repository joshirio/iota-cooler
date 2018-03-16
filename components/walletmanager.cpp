#include "walletmanager.h"
#include "../utils/definitionholder.h"
#include "../utils/Qt-AES/qaesencryption.h"

#include <QtCore/QtGlobal>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

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

void WalletManager::createAndInitWallet(const QString &filePath)
{
    checkLock();

    QFile destFile(filePath);
    QString errorMessage;

    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorMessage = tr("Failed to open create file %1: %2")
                .arg(filePath).arg(destFile.errorString());
        emit walletWriteError(errorMessage);
        return;
    }

    //TODO
    QString datachecksumString;
    QString aesIvVector;
    QByteArray aesWalletData;

    //TODO
    datachecksumString = "";
    aesIvVector = "";
    aesWalletData.append("test");

    QTextStream out(&destFile);
    out << m_magicString << "\n";
    out << "version:" << DefinitionHolder::WALLET_VERSION << "\n";
    out << "data_checksum:" << datachecksumString << "\n";
    out << "aes_iv:" << aesIvVector << "\n";
    out << "aes_wallet_data:" << aesWalletData.toBase64();

    destFile.close();
}

void WalletManager::checkLock()
{
    if (m_encryptionKey.isEmpty())
        qFatal("[FATAL ERROR:] encryption key not set, call WalletManager::unlockWallet() first!");
}

WalletManager::WalletManager(QObject *parent) :
    QObject(parent), m_currentWalletOp(NoOp)
{
    m_magicString = "IOTACOOL";
}

WalletManager::~WalletManager()
{

}
