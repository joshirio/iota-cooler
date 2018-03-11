#include "utilsiota.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QString>

bool UtilsIOTA::isValidTxHash(const QString &txHash)
{
    QRegularExpression reg("^[A-Z9]{81}$");
    return reg.match(txHash).hasMatch();
}

UtilsIOTA::UtilsIOTA()
{

}
