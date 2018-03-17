#include "utilsiota.h"
#include "definitionholder.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QString>

QString UtilsIOTA::currentNodeUrl = DefinitionHolder::DEFAULT_NODE;

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

UtilsIOTA::UtilsIOTA()
{

}
