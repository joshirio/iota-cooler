/**
  * \class UtilsIOTA
  * \brief Collection of static utilities and helper functions for IOTA
  * \author Oirio Joshi
  * \date 2018-03-11
  */

#ifndef UTILSIOTA_H
#define UTILSIOTA_H

class QString;

class UtilsIOTA
{
public:
    static bool isValidTxHash(const QString &txHash);
    static QString currentNodeUrl;
private:
    UtilsIOTA();
};

#endif // UTILSIOTA_H
