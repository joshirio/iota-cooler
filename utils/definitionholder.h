/**
  * \class DefinitionHolder
  * \brief This class holds global definitions
  * \author Oirio Joshi
  * \date 2018-03-04
  */

#ifndef DEFINITIONHOLDER_H
#define DEFINITIONHOLDER_H

#include <QtCore/QtGlobal>

class QString;

class DefinitionHolder
{
public:
    static const QString VERSION;        /**< Software version                   */
    static const QString NAME;           /**< Software name                      */
    static const QString COPYRIGHT;      /**< Software copyright                 */
    static const QString ORG;            /**< Organization name used in settings */
    static const QString UPDATE_URL;     /**< Url where to check for updates     */
    static const QString DOWNLOAD_URL;   /**< Url where to download the software */
    static const QString HELP_URL;       /**< Url to help or wiki pages          */
    static const int SOFTWARE_BUILD;     /**< Build no. of the software          */
    static const bool APP_STORE;         /**< Deployment target is an app store  */
    static const bool APPIMAGE_LINUX;    /**< Deployment target is an AppImage   */
    static const bool WIN_PORTABLE;      /**< Deployment is portable Windows app */
    static const QString DEFAULT_NODE;   /**< Default IOTA node for IRI requests */
    static const quint32 WALLET_VERSION; /**< Wallet file version number         */
    static const QString DONATE_URL;     /**< Donation URL                       */

private:
    DefinitionHolder() {} //static only
};

#endif // DEFINITIONHOLDER_H
