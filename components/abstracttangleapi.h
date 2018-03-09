/**
  * \class AbstractTangleAPI
  * \brief This is the abstract base class for the IOTA API implementations.
  * Initially, one based on smidgen (bitifnex.com's multisig wallet) is implemented
  * but in future a more advanced implementation based on the C++ or python API could
  * replace it.
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef ABSTRACTTANGLEAPI_H
#define ABSTRACTTANGLEAPI_H

#include <QObject>

class AbstractTangleAPI : public QObject
{
    Q_OBJECT
public:
    explicit AbstractTangleAPI(QObject *parent = nullptr);

    /**
     * @brief The RequestType enum
     */
    enum RequestType {
        NO_REQUEST, /**< Invalid request status */
        GET_BALANCE, /**< Get balance of an address */
        PROMOTE, /**< Promote a transaction */
        REATTACH, /**< Reattach a transaction */
        MULTISIG_TRANSFER /**< make a transfer from a multisig address */
    };

    /**
     * @brief Start an IOTA API request
     * @param request - the API command
     * @param argList - extra command arguments
     */
    virtual void startAPIRequest(RequestType request, const QStringList &argList) = 0;

    /**
     * @brief Stop the current ongoing API request, if any
     */
    virtual void stopCurrentAPIRequest() = 0;

signals:
    /**
     * @brief Current request finished successfully
     * @param request - the API command used
     * @param responseMessage - output of the API request
     */
    void requestFinished(RequestType request, const QString &responseMessage);

    /**
     * @brief Current request failed with error
     * @param request - the API command used
     * @param errorMessage - error output of the API request
     */
    void requestError(RequestType request, const QString &errorMessage);
};

#endif // ABSTRACTTANGLEAPI_H
