/**
  * \class UtilsIOTA
  * \brief Collection of static utilities and helper functions for IOTA
  * \author Oirio Joshi
  * \date 2018-03-11
  */

#ifndef UTILSIOTA_H
#define UTILSIOTA_H

#include <QtCore/QDateTime>
#include <QtCore/QStringList>

class QString;

class UtilsIOTA
{
public:
    /** Check if the transaction hash format is a valid */
    static bool isValidTxHash(const QString &txHash);

    /** Check if seed format is valid */
    static bool isValidSeed(const QString &seed);

    /**
     * Check if the address format is valid */
    static bool isValidAddress(const QString &address);

    /** The current IOTA node URL used for API requests */
    static QString currentNodeUrl;

    /**
     * @brief The DeviceRole enum
     */
    enum DeviceRole {
        Undefined, /**< Undefined role */
        OnlineSigner, /**< Device broadcasts and prepare transactions */
        OfflineSigner /**< Device only signs transactions securely offline */
    };

    /**
     * @brief The Transation struct
     */
    struct Transation {
        QString tailTxHash; /**< First tx in the bundle */
        QString amount; /**< Amount in iota */
        QString spendingAddress; /**< Address with original funds */
        QString receivingAddress; /**< Receiver */
        QString tag; /**< IOTA tag */
        QDateTime dateTime; /**< Date and time of the transaction */
    };

private:
    UtilsIOTA();
};

#endif // UTILSIOTA_H
