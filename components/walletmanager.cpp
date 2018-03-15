#include "walletmanager.h"

#include <QtCore/QtGlobal>

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

    //TODO
}

void WalletManager::checkLock()
{
    if (m_encryptionKey.isEmpty())
        qFatal("[FATAL ERROR:] encryption key not set, call WalletManager::unlockWallet() first!");
}

WalletManager::WalletManager(QObject *parent) :
    QObject(parent), m_currentWalletOp(NoOp)
{

}

WalletManager::~WalletManager()
{

}
