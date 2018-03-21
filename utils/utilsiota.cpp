#include "utilsiota.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QString>

//later initialized with DefinitionHolder::DEFAULT_NODE to avoid static init dep cycle and crash
QString UtilsIOTA::currentNodeUrl = "";

bool UtilsIOTA::isValidTxHash(const QString &txHash)
{
    QRegularExpression reg("^[A-Z9]{81}$");
    return reg.match(txHash).hasMatch();
}

bool UtilsIOTA::isValidSeed(const QString &seed)
{
    //at this time only seed of length 81 are supported
    QRegularExpression reg("^[A-Z9]{81}$");
    return reg.match(seed).hasMatch();
}

bool UtilsIOTA::isValidAddress(const QString &address)
{
    QRegularExpression reg("^[A-Z9]{81}$|^[A-Z9]{90}$"); //81 or 90 (checksum) length
    //checksum is not checked at this time
    return reg.match(address).hasMatch();
}

UtilsIOTA::UtilsIOTA()
{

}
