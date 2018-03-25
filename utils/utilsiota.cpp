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
    //TODO: if 90 length, check checksum
    return reg.match(address).hasMatch();
}

QString UtilsIOTA::getEasyReadableTag(const QString &tag)
{
    QString t = tag;
    int lastCharIndex, i = 0;
    foreach (QChar c, t) {
        if (c != '9') {
            lastCharIndex = i;
        }
        i++;
    }
    if (lastCharIndex < t.length()) {
        t.truncate(lastCharIndex+1);
    }

    return t;
}

UtilsIOTA::UtilsIOTA()
{

}
