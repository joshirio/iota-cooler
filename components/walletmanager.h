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
#include "../utils/utilsiota.h"

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

    /**
     * @brief The WalletError class, containing error related info
     */
    class WalletError {
    public:
        /**
        * @brief The WalletError enum
        */
        enum ErrorType {
            NoError, /**< No error occurred */
            WalletReadError, /**< Error occurred while reading the wallet file.
                         This could be cause by a wrong encryption key or a corrupt file. */
            WalletWriteError, /**< Error occurred while writing the wallet file.
                               This could be caused by missing permissions or a damaged file system. */
            WalletFileParsingError, /**< Error occurred while parsing the JSON wallet string.
                                     Possible cause is a wrong encryption passphrase or corrupted data. */
            WalletInvalidPassphrase /**< Passphrase to decrypt the wallet is not valid.
                                     This is only a guess based on JSON error 'illegal value'. */
        };

        ErrorType errorType = NoError;
        QString errorString = "";
    };

    static WalletManager& getInstance();
    static void destroy();

    /**
     * @brief Get the path of the multisig file,
     * which is the file with signed or yet to sign transactions
     * that is embedded into the encrypted wallet file.
     * In case of smidgen tangleAPI implementation this is
     * just the smidgen mutisig file.
     * @return file path in a temp directory
     */
    static QString getTmpMultisigSignFilePath();

    /**
     * @brief Deletes the multisig file
     * @return true on success
     */
    static bool deleteTmpMultisifSignFile();

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
     * @brief Get the wallet passphrase
     * @return wallet password
     */
    QString getCurrentWalletPassphrase();

    /**
     * @brief Create and initialize a new wallet file.
     * Writes the wallet to file and sets the current wallet op
     * to InitOffline for the next step which is offline init.
     * Emits walletWriteError()
     * @param filePath - the new wallet file saving path
     * @param recoverFunds - true if current op args should indicate the intention
     * to recover funds from previous seeds (only an intention marker)
     * @param error - WalletError, containing error info on failure
     * @return bool - true on success
     */
    bool createAndInitWallet(const QString &filePath, bool recoverFunds,
                             WalletError &error);

    /**
     * @brief Save and write internal wallet structure to an encrypted file
     * Emits walletWriteError()
     * @param filePath - file saving path
     * @param error - WalletError, containing error info on failure
     * @return bool - true on success
     */
    bool writeWalletToFile(const QString &filePath, WalletError &error);

    /**
     * @brief Read and load encrypted wallet file
     * Emits walletReadError()
     * @param filePath - wallet file to read
     * @param error - WalletError, containing error info on failure
     * @return bool - true on success
     */
    bool readWalletFile(const QString &filePath, WalletError &error);

    /**
     * @brief Get the current wallet operation, ie. the next step.
     * NoOp means wallet is in a clean state with no unfinished jobs.
     * @return wallet operation enum
     */
    WalletOp getCurrentWalletOp();

    /**
     * @brief Import the temporary multisig file to be embedded into the
     * wallet, call this before writing the wallet file.
     * @return true on success
     */
    bool importMultisigFile();

    /**
     * @brief Import the temporary multisig file to be embedded into the
     * wallet as a clean state (ie. no pending multisig ops like transfers).
     * Call this before writing the wallet file.
     * @return true on success
     */
    bool importMultisigFileAsCleanBackup();

    /**
     * @brief Export the multisig file as a temporary file.
     * Call this only after a wallet has been read.
     * @return true on success
     */
    bool exportMultisigFile();

    /**
     * @brief Export the clean state (no pending op) multisig file as a temporary file.
     * Call this only after a wallet has been read.
     * @return true on success
     */
    bool exportCleanBackupMultisigFile();

    /**
     * @brief Clear the internal stored multisig file
     */
    void clearImportedMultisigFile();

    /**
     * @brief Clear the internal clean backup stored multisig file
     */
    void clearCleamBackupImportedMultisigFile();

    /**
     * @brief Save current wallet to disk
     * @param walletFilePath - file where wallet is written to
     * @param importTmpMultisigFile - if true, importMultisigFile() is called
     * which sets the temp multisig file as current one
     * @param importTmpCleanBackupMultisigFile - if true, importMultisigFileAsCleanBackup() is called
     * which sets the temp multisig file as current clean state multisig backup
     * @param error - WalletError, containing error info on failure
     * @return true on success
     */
    bool saveWallet(const QString &walletFilePath,
                    bool importTmpMultisigFile,
                    bool importTmpCleanBackupMultisigFile,
                    WalletError &error);

    /**
     * @brief Restore current wallet from disk
     * @param walletFilePath - file where wallet is read from
     * @param exportTmpMultisigFile - if true, exportMultisigFile() is called
     * which writes the temp multisig file from current one
     * @param exportTmpCleanBackupMultisigFile - if true, exportMultisigFileAsCleanBackup() is called
     * which writes the temp multisig file from current clean state multisig backup
     * @param error - WalletError, containing error info on failure
     * @return true on success
     */
    bool restoreWallet(const QString &walletFilePath,
                       bool exportTmpMultisigFile,
                       bool exportTmpCleanBackupMultisigFile,
                       WalletError &error);

    /**
     * @brief Set the current wallet operation (next required step)
     * @param op - the operation (enum)
     * @param opArgs - operation arguments, if any
     */
    void setCurrentWalletOp(WalletOp op, const QVariantList &opArgs);

    /**
     * @brief Get the current wallet operation (next required step)
     * @param opArgs - where (optional) op args are saved to
     * @return wallet op (enum)
     */
    WalletOp getCurrentWalletOp(QVariantList &opArgs);

    /**
     * @brief Get past used addresses
     * @return QVariant list with the addresses
     */
    QVariantList getPastUsedAddresses();

    /**
     * @brief Add an address to the list of past used addresses
     * @param address to add
     */
    void addPastUsedAddress(const QString &address);

    /**
     * @brief Get current receiving address
     * @return address string
     */
    QString getCurrentAddress();

    /**
     * @brief Set the current receiving address
     * @param address
     */
    void setCurrentAddress(const QString &address);

    /**
     * @brief Set the online seed for hot signing
     * @param seed
     */
    void setOnlineSeed(const QString &seed);

    /**
     * @brief Get the online (hot signing) seed
     * @return seed
     */
    QString getOnlineSeed();

    /**
     * @brief Add a transaction entry to the past transaction object array.
     * If a transaction has multiple receivers, please add same tx multiple times
     * with updated amounts and receivers but same tail tx hash.
     * @param transaction (struct)
     */
    void addPastSpendingTx(const UtilsIOTA::Transation &transaction);

    /**
     * @brief Get past spending transactions
     * @return list of transaction structs
     */
    QList<UtilsIOTA::Transation> getPastSpendingTxs();

    /**
     * @brief Make a clean state (no pending txs) backup of the multisig file
     */
    void backupMultisigFileAsClean();

    /**
     * @brief Restore clean multisig file backup
     */
    void restoreCleanMultisigFileBackup();

    /** Get the raw json data */
    QString getRawJsonData() const;

private:
    explicit WalletManager(QObject *parent = nullptr);
    WalletManager(const WalletManager&) : QObject(0) {}
    ~WalletManager();

    /**
     * @brief Make sure wallet is unlocked, if not crash with error
     */
    void checkLock() const;

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
     * @param error - WalletError, containing error info on failure
     * @return bool - true on successfull json conversion
     */
    bool decryptAndDeserializeWallet(const QByteArray &aesData,
                                     const QByteArray &iv,
                                     WalletError &error);

    static WalletManager *m_instance;
    WalletOp m_currentWalletOp;
    QString m_encryptionKey;
    QString m_magicString;
    QJsonObject m_jsonObject;
};

#endif // WALLETMANAGER_H
