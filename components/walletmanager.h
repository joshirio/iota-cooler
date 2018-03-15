/**
  * \class WalletManager
  * \brief This class handles everything related to the wallet file, like
  * writing/reading the wallet file to/from disk, encrypting/decrypting
  * sensible data and state updates. The wallet file format is based on JSON,
  * see doc/wallet/format.md for specifics.
  * \author Oirio Joshi
  * \date 2018-03-15
  */

#ifndef WALLETMANAGER_H
#define WALLETMANAGER_H

#include <QObject>

class WalletManager : public QObject
{
    Q_OBJECT

public:
    static WalletManager& getInstance();
    static void destroy();

private:
    explicit WalletManager(QObject *parent = nullptr);
    WalletManager(const WalletManager&) : QObject(0) {}
    ~WalletManager();

    static WalletManager *m_instance;
};

#endif // WALLETMANAGER_H
