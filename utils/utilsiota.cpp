#include "utilsiota.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>

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
    int lastCharIndex, i;
    lastCharIndex = i = 0;
    foreach (QChar c, t) {
        if (c != '9') {
            lastCharIndex = i;
        }
        i++;
    }
    if (lastCharIndex < t.length()) {
        t.truncate(lastCharIndex+1);
    }
    if (lastCharIndex == 0) {
        t = "";
    }

    return t;
}

UtilsIOTA::UtilsIOTA()
{

}

QList<UtilsIOTA::Transation> UtilsIOTA::parseAddrTransfersQuickJson(const QString &jsonString)
{
    QList<UtilsIOTA::Transation> list;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error == QJsonParseError::NoError) {
        QJsonArray txsJsonArray = doc.array();

        //used to skip reattachments
        QStringList bundleHashes;

        foreach (QJsonValue jTx, txsJsonArray) {
            QJsonObject jObj = jTx.toObject();
            UtilsIOTA::Transation tx;
            tx.tailTxHash = jObj.value("hash").toString(); //not real tail hash (quick version)
            tx.amount = jObj.value("value").toVariant().toString();
            //addresses not available in quick version
            qint64 amount = tx.amount.toLongLong();
            tx.spendingAddress = (amount < 0) ? jObj.value("address").toString()
                                              : QObject::tr("view on tangle explorer");
            tx.receivingAddress = (amount >= 0) ? jObj.value("address").toString()
                                               : QObject::tr("view on tangle explorer");
            tx.tag = jObj.value("tag").toString();
            QDateTime d;
            d.setSecsSinceEpoch(jObj.value("timestamp").toVariant().toString().toLongLong());
            tx.dateTime = d;

            //add only if not yet seen (skip reattachments)
            QString bundle = jObj.value("bundle").toString();
            if (!bundleHashes.contains(bundle)) {
                list.append(tx);
                bundleHashes.append(bundle);
            }
        }
    }

    return list;
}
