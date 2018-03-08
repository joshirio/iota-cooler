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

    enum RequestType {
        GET_BALANCE,
        PROMOTE,
        REATTACH,
        MULTISIG_TRANSFER
    };

signals:

};

#endif // ABSTRACTTANGLEAPI_H
