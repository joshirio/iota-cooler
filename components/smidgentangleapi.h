/**
  * \class SmidgenTangleAPI
  * \brief IOTA API implementation based on smidgen (bitfinex.com's multisig wallet).
  * \author Oirio Joshi
  * \date 2018-03-08
  */

#ifndef SMIDGENTANGLEAPI_H
#define SMIDGENTANGLEAPI_H

#include "abstracttangleapi.h"

class SmidgenTangleAPI : public AbstractTangleAPI
{
    Q_OBJECT
public:
    explicit SmidgenTangleAPI(QObject *parent = nullptr);

    void startAPIRequest(RequestType request, const QStringList &argList);
    void stopCurrentAPIRequest();

private:
    RequestType m_currentRequest;
};

#endif // SMIDGENTANGLEAPI_H
