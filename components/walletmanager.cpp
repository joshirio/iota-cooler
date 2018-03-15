#include "walletmanager.h"

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

WalletManager::WalletManager(QObject *parent) :
    QObject(parent)
{

}

WalletManager::~WalletManager()
{

}
