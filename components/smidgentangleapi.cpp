#include "smidgentangleapi.h"
#include "../utils/definitionholder.h"
#include "../utils/utilsiota.h"
#include "walletmanager.h"

#include <QApplication>

SmidgenTangleAPI::SmidgenTangleAPI(QObject *parent) :
    AbstractTangleAPI(parent), m_currentRequest(RequestType::NoRequest)
{
    m_process = new QProcess(this);

    //use tmp file as smidgen multisig wallet file
    m_smidgenMultisigFilePath = WalletManager::getTmpMultisigSignFilePath();

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &SmidgenTangleAPI::processReadyReadOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &SmidgenTangleAPI::processReadyReadOutput);
    connect(m_process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this, &SmidgenTangleAPI::processError);
    connect(m_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &SmidgenTangleAPI::processFinished);
}

SmidgenTangleAPI::~SmidgenTangleAPI()
{

}

void SmidgenTangleAPI::startAPIRequest(RequestType request, const QStringList &argList)
{
    //save original request args
    m_requestArgs = argList;

    QString command;
    QStringList args;
    QString smidgenPath;
    QString seed;

#ifdef Q_OS_WIN
    smidgenPath =  QApplication::applicationDirPath().append("/iotacooler-smidgen.exe");
#endif
#ifdef Q_OS_OSX
    smidgenPath = QApplication::applicationDirPath().append("/iotacooler-smidgen");
#endif
#ifdef Q_OS_LINUX
    smidgenPath = "/usr/bin/iotacooler-smidgen";
    if (DefinitionHolder::SNAP_PKG) {
        smidgenPath.prepend("/snap/iotacooler/current");
    } else if (DefinitionHolder::APPIMAGE_LINUX) {
        smidgenPath = QApplication::applicationDirPath().append("/iotacooler-smidgen");
    } else if (DefinitionHolder::DEB_LINUX) {
        smidgenPath.prepend("/opt/iotacooler/usr/bin");
    }
#endif

    //init command and etra args
    QStringList extraArgs; //extra arguments for specific command
    m_currentRequest = request;
    switch (m_currentRequest) {
    case NoRequest:
        //skip
        break;
    case Promote:
        command = "promote";
        if (argList.size() > 0)
            extraArgs.append(argList.at(0)); //tail tx hash
        break;
    case Reattach:
        command = "reattach";
        if (argList.size() > 0)
            extraArgs.append(argList.at(0)); //tail tx hash
        break;
    case CreateSeed:
        command = "generate-seed";
        break;
    case CreateMultisigWallet:
        if (!clearSmidgenMultisigFile()) return;
        seed = argList.at(1);
        command = "multisig";
        extraArgs.append("create");
        extraArgs.append(argList.at(0)); //signing party name
        extraArgs.append(m_smidgenMultisigFilePath); //smidgen multisig wallet file
        extraArgs.append("--no-validation"); //skip check to allow offline usage
        break;
    case AddMultisigParty:
        seed = argList.at(1);
        command = "multisig";
        extraArgs.append("add");
        extraArgs.append(argList.at(0)); //signing party name
        extraArgs.append(m_smidgenMultisigFilePath); //smidgen multisig wallet file
        extraArgs.append("--no-validation"); //skip check to allow offline usage
        break;
    case FinalizeMultsigWallet:
        command = "multisig";
        extraArgs.append("finalize");
        extraArgs.append(m_smidgenMultisigFilePath); //smidgen multisig wallet file
        break;
    case GetBalance:
        command = "get-balance";
        extraArgs.append(argList.at(0));
        break;
    case MultisigTransfer:
    {
        QStringList transferList = argList;
        command = "multisig";
        extraArgs.append("transfer");
        QString tag = transferList.takeLast();
        seed = transferList.takeLast();
        QString balance = transferList.takeLast();
        QString id = transferList.takeLast();
        extraArgs.append(transferList);
        extraArgs.append(id);
        extraArgs.append(m_smidgenMultisigFilePath); //smidgen multisig wallet file
        extraArgs.append("--balance");
        extraArgs.append(balance);
        extraArgs.append("--tag");
        extraArgs.append(tag);
    }
        break;
    case RecoverFundsSign:
    {
        QString addressKeyIndex = argList.at(0);
        QString receivingAddr = argList.at(1);
        QString balance = argList.at(2);
        command = "iotacooler";
        extraArgs.append("recover-funds-sign");
        extraArgs.append(addressKeyIndex);
        extraArgs.append(receivingAddr);
        extraArgs.append(balance);
    }
        break;
    case RecoverFundsSend:
    {
        command = "iotacooler";
        extraArgs.append("recover-funds-send");
        extraArgs.append(argList.at(0));
        extraArgs.append(argList.at(1));
    }
        break;
    case GenerateAddresses:
    {
        QString startIndex = argList.at(0);
        QString endIndex = argList.at(1);
        command = "iotacooler";
        extraArgs.append("get-addresses");
        extraArgs.append(startIndex);
        extraArgs.append(endIndex);
    }
        break;
    case IsAddressSpent:
        command = "iotacooler";
        extraArgs.append("is-address-spent");
        extraArgs.append(argList.at(0));
        break;
    case GetAddrTransfersQuick:
        command = "iotacooler";
        extraArgs.append("get-address-transfers-quick");
        extraArgs.append(argList.at(0));
        break;
    }

    //init args
    args.append(command);
    args.append(extraArgs);
    args.append("--provider");
    args.append(UtilsIOTA::currentNodeUrl);

    m_processOutput.clear();
    m_process->start(smidgenPath, args);

    //if seed arg is required, use interactive input to avoid seed leak
    //also used for other interactive inputs like addresses
    if ((m_currentRequest == CreateMultisigWallet) ||
            (m_currentRequest == AddMultisigParty) ||
            (m_currentRequest == MultisigTransfer)) {
        if (!seed.isEmpty()) {
            m_process->waitForReadyRead();
            m_process->write(seed.toLatin1());
            m_process->waitForBytesWritten();
        }
    } else if (m_currentRequest == GenerateAddresses) {
        QString onlineSeed = argList.at(2);
        QString offlineSeed = argList.at(3);
        QString combinedSeeds = onlineSeed + ":" + offlineSeed;
        m_process->waitForReadyRead();
        m_process->write(combinedSeeds.toLatin1());
        m_process->waitForBytesWritten();
    } else if (m_currentRequest == RecoverFundsSign) {
        QString onlineSeed = argList.at(3);
        QString offlineSeed = argList.at(4);
        QString combinedSeeds = onlineSeed + ":" + offlineSeed;
        m_process->waitForReadyRead();
        m_process->write(combinedSeeds.toLatin1());
        m_process->waitForBytesWritten();
    }

    //close write channel to allow
    //ready output signals, see docs
    m_process->closeWriteChannel();
}

void SmidgenTangleAPI::stopCurrentAPIRequest()
{
    if (m_currentRequest == RequestType::NoRequest) return;

    if (m_process->state() != QProcess::NotRunning) {
        //disable crash signal errors
        this->blockSignals(true);

        //stop process
        m_process->kill();
        m_process->waitForFinished();

        this->blockSignals(false);

        m_currentRequest = NoRequest;
    }
}

void SmidgenTangleAPI::processError(QProcess::ProcessError error)
{
    //if process has been killed (request is cleared)
    if (m_currentRequest == NoRequest) return;

    RequestType request = m_currentRequest;
    QString errorMessage;

    switch (error) {
    case QProcess::FailedToStart:
        errorMessage = tr("iotacooler-smidgen process failed to start.<br /><br />"
                          "Please <b>install</b> iotacooler-smidgen by visiting "
                          "<a href='https://github.com/joshirio/iota-cooler-smidgen'>"
                          "https://github.com/joshirio/iota-cooler-smidgen</a>.<br />");
        break;
    case QProcess::Crashed:
        errorMessage = tr("iotacooler-smidgen process crashed");
        break;
    case QProcess::WriteError:
        errorMessage = tr("iotacooler-smidgen process write error");
        break;
    case QProcess::ReadError:
        errorMessage = tr("iotacooler-smidgenprocess read error");
        break;
    case QProcess::UnknownError:
        errorMessage = tr("Unknown error during iotacooler-smidgen process");
        break;
    default:
        errorMessage = tr("Unknown error during iotacooler-smidgen "
                          "(in switch default)");
        break;
    }

    //clean request args
    m_requestArgs.clear();
    m_currentRequest = NoRequest;

    emit requestError(request, errorMessage);
}

void SmidgenTangleAPI::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    //copy request args and clean them for future requests
    QStringList requestArgs = m_requestArgs;
    m_requestArgs.clear();
    RequestType request = m_currentRequest;
    m_currentRequest = NoRequest;

    bool error = false;
    QString errorMessage, message;

    if (exitStatus == QProcess::NormalExit) {
        QString result = m_processOutput;

        switch (request) {
        case Promote:
            if (result.contains("successfully")) {
                message = tr("Transaction successfully promoted!");
            } else if (result.contains("too old")) {
                errorMessage = tr("Transaction too old, try to reattach it!");
                error = true;
            } else if (result.contains("not a tail")) {
                errorMessage = tr("The entered transaction hash in not a tail transaction hash!"
                                  "<br />Use <a href='https://thetangle.org'>thetangle.org</a> "
                                  "to find the hash of the first transaction (bundle index 0).");
                error = true;
            } else if (result.contains("Invalid transaction")) {
                errorMessage = tr("The entered transaction hash in not a valid transaction!");
                error = true;
            } else if (result.contains("No valid transaction hash given")) {
                errorMessage = tr("The entered data is not a valid transaction hash!"
                                  "<br />Allowed format: 81 long character string of A-Z and 9");
                error = true;
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case Reattach:
            if (result.contains("Successfully")) {
                QString bundleHash = "";
                QStringList s = result.split("Bundle:", QString::SkipEmptyParts);
                if (s.size() >= 2) {
                    bundleHash = s.at(1);
                    bundleHash = bundleHash.trimmed();
                }
                message = tr("Transaction successfully reattached!"
                             "<br /> New bundle hash: "
                             "<a href='https://thetangle.org/bundle/%1'>%1</a>")
                        .arg(bundleHash);
            } else if (result.contains("Invalid tail")) {
                errorMessage = tr("The entered transaction hash in not a tail transaction hash!"
                                  "<br />Use <a href='https://thetangle.org'>thetangle.org</a> "
                                  "to find the hash of the first transaction (bundle index 0).");
                error = true;
            } else if (result.contains("Invalid Bundle")) {
                errorMessage = tr("The entered transaction hash in not a valid transaction!");
                error = true;
            } else if (result.contains("No valid transaction hash given")) {
                errorMessage = tr("The entered data is not a valid transaction hash!"
                                  "<br />Allowed format: 81 long character string of A-Z and 9");
                error = true;
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case CreateSeed:
            if (UtilsIOTA::isValidSeed(result.simplified())) {
                //return seed as seedname(arg):seed
                if (requestArgs.isEmpty())
                    requestArgs.append("unnamed");
                message = QString("Generated:").append(
                            requestArgs.at(0)).append(":").append(result.simplified());
            } else {
                errorMessage = tr("Invalid seed: ").append(result);
                error = true;
            }
            break;
        case CreateMultisigWallet:
            if (result.contains("Successfully")) {
                //return ok as OK:party_name
                message = QString("Multisig create:OK:").append(requestArgs.at(0));
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case AddMultisigParty:
            if (result.contains("Successfully")) {
                //return ok as OK:party_name
                message = QString("Multisig add:OK:").append(requestArgs.at(0));
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case FinalizeMultsigWallet:
            if (result.contains("Successfully")) {
                //return ok as OK:main_address
                int offset = 1; //position of address
#ifdef Q_OS_WIN
                //on windows add 1 to the offset because of C:// (having a colon)
                offset++;
#endif
                QString mainAddress = result.simplified().split(":").at(offset).trimmed();
                message = QString("Multisig finalize:OK:").append(mainAddress);
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case GetBalance:
            if (result.contains("Balance:")) {
                //return ok as Balace:address:iotabalance:readablebalance
                QString ibalance = result;
                ibalance.remove("Balance: ");
                QString readableBalance = ibalance.simplified().trimmed();
                ibalance = ibalance.split(" (").at(0);
                message = QString("Balance:").append(requestArgs.at(0))
                        .append(":" + ibalance.append(":"))
                        .append(readableBalance);
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case MultisigTransfer:
            if (result.contains("Successfully signed transfer")) {
                if (requestArgs.contains("offline")) {
                    message = "SignOK:offline";
                }
            } else if (result.contains("Transaction sent")) {
                //return ok as TransferOK:tailTxHash:newAddress
                message = "TransferOK:";
                QString txHash = result.split("hash: ").at(1);
                txHash = txHash.split("\ninfo").at(0);
                QString newAddress = result.split("New main address: ").at(1);
                newAddress.remove("\n");
                message.append(txHash).append(":").append(newAddress);
            } else if (result.contains("Bundle is invalid")) {
                error = true;
                errorMessage = tr("Invalid bundle: please check your seed, "
                                  "balance and addresses.<br />"
                                  "If this error persists, consider aborting"
                                  " the transaction and start over again.");
            } else if (result.contains("addresses were used already")) {
                error = true;
                errorMessage = QString("Address reuse detected!<br />") //no tr() bc it's parsed
                        .append(result);
            } else {
                //unexpected response
                errorMessage = result;
                error = true;
            }
            break;
        case GenerateAddresses:
            if (result.contains("Done:")) {
                QString addresses;
                QStringList l = result.split(";", QString::SkipEmptyParts);
                l.removeFirst(); //rm input prompt
                l.removeLast(); //rm last newline (\n) char
                foreach (QString s, l) {
                    QString sf = s.split(":").at(2);
                    addresses.append(sf.append(":"));
                }
                message = "GenAddr:OK:" + addresses;
            }  else {
                errorMessage = result;
                error = true;
            }
            break;
        case IsAddressSpent:
            if (result.contains("AddressSpent")) {
                message = result;
            } else {
                errorMessage = result;
                error = true;
            }
            break;
        case RecoverFundsSign:
            if (result.contains("SignValid:true:")) {
                message = result.split("SignValid:true: ", QString::SkipEmptyParts).at(1);
            } else if (result.contains("SignValid:false")) {
                errorMessage = tr("Bundle signature invalid!");
                error = true;
            } else {
                errorMessage = result;
                error = true;
            }
            break;
        case RecoverFundsSend:
            if (result.contains("Transaction sent")) {
                //return ok as RecoveryOK:tailTxHash
                message = "RecoveryOK:";
                QString txHash = result.split("hash: ").at(1);
                txHash = txHash.split("\ninfo").at(0);
                message.append(txHash);
            } else {
                errorMessage = result;
                error = true;
            }
            break;
        case GetAddrTransfersQuick:
            if (result.contains("Transfers:")) {
                //return ok as Transfers:address:jsondata
                message = result;
            } else {
                errorMessage = result;
                error = true;
            }
            break;
        default:
            //unexpected response
            errorMessage = result;
            error = true;
            break;
        }
    } else {
        //unexpected response
        errorMessage = tr("Process finished with non zero exit code (crash).");
        error = true;
    }

    if (error) {
        emit requestError(request, errorMessage);
    } else {
        emit requestFinished(request, message);
    }

}

void SmidgenTangleAPI::processReadyReadOutput()
{
    QString newOutput;
    newOutput.append(m_process->readAllStandardError());
    newOutput.append(m_process->readAllStandardOutput());
    m_processOutput.append(newOutput);

    switch (m_currentRequest) {
    case GenerateAddresses:
        if (newOutput.contains("Done:")) {
            QStringList progressList = newOutput.split(":").at(1).split("-");
            int currentIndex = progressList.at(0).toInt();
            int endIndex = progressList.at(1).toInt();
            emit addressGenerationProgress(currentIndex, endIndex);
        }
        break;
    default:
        break;
    }
}

bool SmidgenTangleAPI::clearSmidgenMultisigFile()
{
    if (!WalletManager::deleteTmpMultisifSignFile()) {
        emit requestError(m_currentRequest,
                          tr("Failed to remove temporary multisig file!"));
        return false;
    }
    return true;
}
