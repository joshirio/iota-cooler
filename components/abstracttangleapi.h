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
        NoRequest, /**< Invalid request status */
        CreateSeed, /**< Generate a seed */
        GetBalance, /**< Get balance of an address */
        GetAddrTransfersQuick, /**< Get history of an address,
                                 * quick version has no addresses or tail tx hash
                                 * but is fast
                                 */
        GenerateAddresses, /**< Generate multisig addresses */
        IsAddressSpent, /**< Whether an address had outgoing
                             transfers (also pre snapshot) */
        Promote, /**< Promote a transaction */
        Reattach, /**< Reattach a transaction */
        MultisigTransfer, /**< Make a transfer from a multisig address */
        CreateMultisigWallet, /**< Create a multisig file with 1 signing party */
        AddMultisigParty, /**< Add a signing party to the multisig file.
                               Can only be used until multisig file finalization */
        FinalizeMultsigWallet, /**< Finalize mutisig file */
        RecoverFundsSign, /**< Sign a tx to recover funds from a multisig address */
        RecoverFundsSend /**< Send a signed tx to recover funds, after RecoverFundsSign */
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
    //NOTE: because of lazyness each request doesn't have a custom response signal
    //instead checkout smidgen implementation for parsing arguments in the
    //responseMessage. Might change this in future if an alternative impl is needed

    /**
     * @brief Current request failed with error
     * @param request - the API command used
     * @param errorMessage - error output of the API request
     */
    void requestError(RequestType request, const QString &errorMessage);

    /**
     * @brief Progress signal informing about current step and total steps
     * in multisig address generation process (GenerateAddresses)
     * @param currentIndex - current progress (current step)
     * @param endIndex - last address index (last step), tot steps = endIndex + 1
     */
    void addressGenerationProgress(int currentIndex, int endIndex);
};

#endif // ABSTRACTTANGLEAPI_H
