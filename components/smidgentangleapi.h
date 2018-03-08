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
};

#endif // SMIDGENTANGLEAPI_H
