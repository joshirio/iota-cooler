/**
  * \class DefinitionHolder
  * \brief This class holds global definitions
  * \author Oirio Joshi
  * \date 2018-03-04
  */

#ifndef DEFINITIONHOLDER_H
#define DEFINITIONHOLDER_H

class QString;

class DefinitionHolder
{
public:
    static QString VERSION;        /**< Software version                   */
    static QString NAME;           /**< Software name                      */
    static QString COPYRIGHT;      /**< Software copyright                 */
    static QString ORG;            /**< Organization name used in settings */
    static QString UPDATE_URL;     /**< Url where to check for updates     */
    static QString DOWNLOAD_URL;   /**< Url where to download the software */
    static QString HELP_URL;       /**< Url to help or wiki pages          */
    static int SOFTWARE_BUILD;     /**< Build no. of the software          */
    static bool APP_STORE;         /**< Deployment target is an app store  */
    static bool APPIMAGE_LINUX;    /**< Deployment target is an AppImage   */
    static bool WIN_PORTABLE;      /**< Deployment is portable Windows app */
    static QString DEFAULT_NODE;   /**< Default IOTA node for IRI requests */

private:
    DefinitionHolder() {} //static only
};

#endif // DEFINITIONHOLDER_H
