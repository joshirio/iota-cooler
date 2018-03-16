/**
  * \class WalletManager
  * \brief This class handles everything related to the wallet file, like
  * writing/reading the wallet file to/from disk, encrypting/decrypting
  * sensible data and state updates. The wallet file format is based on JSON,
  * see doc/wallet/format.md for specifics.
  * Before calling any member funtion it is required to unlock the wallet
  * by using WalletManager::unlock().
  * \author Oirio Joshi
  * \date 2018-03-15
  */

#ifndef WALLETMANAGER_H
#define WALLETMANAGER_H

#include <QObject>
#include <QJsonObject>

class WalletManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief The wallet operations enum
     */
    enum WalletOp {
        NoOp, /**< Nothing to do */
        InitOffline, /**< Init new wallet file on offline device */
        ColdSign, /**< Sign transaction on offline device */
        HotSign, /**< Sign transaction on online device and broadcast it */
        RecoverOffline, /**< Recover wallet from seeds, offline step */
        RecoverOnline /**< Recover wallet from seeds, online step */
    };

    static WalletManager& getInstance();
    static void destroy();

    /**
     * @brief Unlock wallet by caching the encryption passphrase
     * @param encryptionKey - the aes encryption key used to read/write the wallet file
     */
    void unlockWallet(const QString &encryptionKey);

    /**
     * @brief Remove passphrase from cache
     */
    void lockWallet();

    /**
     * @brief Create and initialize a new wallet file.
     * Writes the wallet to file and sets the current wallet op
     * to InitOffline for the next step which is offline init.
     * Emits walletWriteError()
     * @param filePath - the new wallet file saving path
     * @return bool - true on success
     */
    bool createAndInitWallet(const QString &filePath);

    /**
     * @brief Save and write internal wallet structure to an encrypted file
     * Emits walletWriteError()
     * @param filePath - file saving path
     * @return bool - true on success
     */
    bool writeWalletToFile(const QString &filePath);

    /**
     * @brief Read and load encrypted wallet file
     * Emits walletReadError()
     * @param filePath - wallet file to read
     * @return bool - true on success
     */
    bool readWalletFile(const QString &filePath);

signals:
    /**
     * @brief Emitted if an error occurred while reading the wallet file.
     * This could be cause by a wrong encryption key or a corrupt file.
     * @param message - the error message
     */
    void walletReadError(const QString &message);

    /**
     * @brief Emitted if an error occurred while writing the wallet file.
     * This could be caused by missing permissions or a damaged file system.
     * @param message - the error message
     */
    void walletWriteError(const QString &message);

    /**
     * @brief Emitted if an error occurred while parsing the wallet file.
     * Possible cause is a wrong encryption passphrase or corrupted data.
     * @param message - the error message
     */
    void walletFileParsingError(const QString &message);

private:
    explicit WalletManager(QObject *parent = nullptr);
    WalletManager(const WalletManager&) : QObject(0) {}
    ~WalletManager();

    /**
     * @brief Make sure wallet is unlocked, if not crash with error
     */
    void checkLock();

    /**
     * @brief Get a random string for the IV vector for CBC AES
     * @return random iv vector as byte array
     */
    QByteArray getRandomIv();

    /**
     * @brief Serialize current wallet in an aes encrypted byte array
     * @param iv - AES (CBC) initialization vector
     * @return byte array
     */
    QByteArray serializeAndEncryptWallet(const QByteArray &iv);

    /**
     * @brief Decrypt and deserialize wallet data to the internal json doc
     * @param aesData - the encrypted data as byte array
     * @param iv - AES (CBC) initialization vector
     * @return QString - the raw data (json) as string
     */
    void decryptAndDeserializeWallet(const QByteArray &aesData,
                                     const QByteArray &iv);

    static WalletManager *m_instance;
    WalletOp m_currentWalletOp;
    QString m_encryptionKey;
    QString m_magicString;
    QJsonObject m_jsonObject;
};

#endif // WALLETMANAGER_H
