#include "smidgentangleapi.h"
#include "../utils/definitionholder.h"

SmidgenTangleAPI::SmidgenTangleAPI(QObject *parent) :
    AbstractTangleAPI(parent), m_currentRequest(RequestType::NoRequest)
{
    m_process = new QProcess(this);

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
    stopCurrentAPIRequest();
}

void SmidgenTangleAPI::startAPIRequest(RequestType request, const QStringList &argList)
{
    QString command;
    QStringList args;
    QString smidgenPath;

#ifdef Q_OS_WIN
    smidgenPath =  "iotacooler-smidgen.exe";
#endif
#ifdef Q_OS_OSX
    smidgenPath = QString("/Applications/IOTAcooler.app/Contents/MacOS/");
    smidgenPath.append("iotacooler-smidgen");
#endif
#ifdef Q_OS_LINUX
    smidgenPath = "./iotacooler-smidgen";
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
    case GetBalance:
        command = "get-balance";
        extraArgs = m_requestArgs;
        //TODO: just an example rm this
        extraArgs.append("RF9TQPFMGIWUTFWYHLVTDAUIKPGEKWIFBVELXVESPIMFPMQPPXHANGGYBJ9THRXRFFHWRQGQSNPVGJBJDXHNKNNSVY");
        extraArgs.append("--provider");
        extraArgs.append(DefinitionHolder::DEFAULT_NODE);
        break;
    }

    //init args
    args.append(command);
    args.append(extraArgs);

    m_processOutput.clear();
    m_process->start(smidgenPath, args);

    //if login command, use interactive command to avoid pass leak
    //instead of command line args
    /*if (m_currentRequest == AuthRequest) {
            QString megaPass = "YSTSEP9AFGGENBAZDGDZRZEXJIORI9WCLEHNAVDKAOZENLFIVHAZ9ZYP9TGDHFWNMQGYGYYMOGHVSELS9";
            m_process->waitForReadyRead();
            m_process->write(megaPass.toLatin1());
            m_process->waitForBytesWritten();
        }*/ //disable while balance adr given

    //close write channel to allow
    //ready output signals, see docs
    m_process->closeWriteChannel();
}

void SmidgenTangleAPI::stopCurrentAPIRequest()
{
    if (m_currentRequest == RequestType::NoRequest) return;

    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
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

    //NOTE: smidgen currently has no progress status output
    /*switch (m_currentRequest) {
    case Promote:
        if (xyz) {
            if (m_processOutput.contains("Promoting:")) {
                emit uploadedChunkReady(m_chunksUploaded, m_totUploadChunks);
                m_chunksUploaded++;
            }
        }
        break;
    default:
        break;
    }*/
}
