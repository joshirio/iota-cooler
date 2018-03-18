/**
  * \class SmidgenTangleAPI
  * \brief IOTA API implementation based on smidgen (bitfinex.com's multisig wallet).
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef SMIDGENTANGLEAPI_H
#define SMIDGENTANGLEAPI_H

#include "abstracttangleapi.h"
#include <QtCore/QProcess>

class SmidgenTangleAPI : public AbstractTangleAPI
{
    Q_OBJECT
public:
    explicit SmidgenTangleAPI(QObject *parent = nullptr);
    ~SmidgenTangleAPI();

    void startAPIRequest(RequestType request, const QStringList &argList);
    void stopCurrentAPIRequest();

private slots:
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processReadyReadOutput();

private:
    bool clearSmidgenMultisigFile();

    RequestType m_currentRequest;
    QProcess *m_process;
    QStringList m_requestArgs;
    QString m_processOutput;
    QString m_smidgenMultisigFilePath;
};

#endif // SMIDGENTANGLEAPI_H
