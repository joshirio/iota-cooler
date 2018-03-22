# Deployment

Build IOTAcooler:

1. Check and update translation files if needed
2. Check DefinitionHolder
3. Check authors description in about dialog, see AUTHORS file
4. Check and update CHANGELOG file
5. Create archive of last commit:
6  git archive -o iotacooler-1.0-src.tar.gz HEAD
7. Uncompress and open project with release config (or just use QtCreator GUI)
8. Compile

Prepare iotacooler-smidgen:
1. Follow: https://github.com/joshirio/iota-cooler-smidgen/blob/master/DEPLOY.md
2. Rename the executable to iotacooler-smidgen (linux/macOS) or iotacooler-smidgen.exe (windows)

Windows:
1. Place iotacooler-smidgen.exe in the same directory as the main exe.
2. Open and compile iotacooler.iss after review
3. The missing .DLL files can be copied from the Qt install or use windeployqt

Windows Portable:
1. Follow similar steps as above
2. Enable DefinitionHolder::WIN_PORTABLE before compiling to use local and portable storage directories
3. Instead of running installer, just create a zip

macOS:
1. Use 'qmake -spec macx-clang iota-cooler.pro' to generate Makefile
2. Move iotacooler-smidgen binary to app boundle in the subfolder where the binary is
3. Use macdeployqt
4. Create a DMG follow this: http://mac101.net/content/how-to/how-to-create-dmg-art-for-fancy-application-installations/
   (alt link: https://web.archive.org/web/20150423212042/http://mac101.net/content/how-to/how-to-create-dmg-art-for-fancy-application-installations/)

Ubuntu:
1. Copy deb dir from installers dir
2. Update DEBIAN/control as needed
3. Copy executable to usr/bin
4. Copy iotacooler-smidgen usr/share/bin/
5. Create deb package
   sudo chown root:root -R deb/
   sudo chmod -R 0755 deb/
   dpkg -b deb/ iotacooler-1.0-x86_64.deb

Arch Linux:
1. Adjust PKGBUILD script from the installers directory
2. copy source tarball to current build directory
3. run updpkgsums to update checksums for source tarball
4. makepkg --printsrcinfo > .SRCINFO 
5. run makepkg -i to build pacman package and to install it

AppImage:
- Use Ubuntu 14.04 LTS (oldest stil supported LTS) for the AppImage creation
- Install missing deps:
   sudo apt install build-essentials mesa-common-dev libglu1-mesa-dev libmysqlclient18 libpq5 libodbc1 libsybdb5
- Download Qt installer from qt.io/download
- Set APPIMAGE_LINUX to true in DefinitionHolder and then compile project in release mode
- Download linuxdeployqt from https://github.com/probonopd/linuxdeployqt
- Download AppImageKit from https://github.com/AppImage/AppImageKit:
   wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
   chmod a+x appimagetool-x86_64.AppImage
- Create myappimg folder on desktop and place files from stuff/deb/ here:
   copy the compiled iota-cooler binary to myappimg/usr/bin
   copy the whole usr/share folder from stuff/deb to myappimg/usr/share
-> Create the AppDir with:
   ./linuxdeployqt-continuous-x86_64.AppImage /home/user/Desktop/myappimg/usr/share/applications/iotacooler.desktop  -bundle-non-qt-libs -no-translations -qmake=/home/user/Qt/5.9.2/gcc_64/bin/qmake
- Integrate the iotacooler-smidgen binary from above (after step above to avoid missing lib errors, iotacooler-smidgen is already self contained):
   move the previously created iotacooler-smidgen binary to myappimg/usr/bin/
- Patch the AppDir to fix an OpenSSL currently affecting Qt's SDK, see https://github.com/Subsurface-divelog/subsurface/issues/779
   sed -i -e 's|1.0.1e|1.0.0\x00|g' /home/user/Desktop/myappimg/usr/lib/libQt5Network.so.5
   for Qt 5.10.1 use: sed -i -e 's|1.0.2k|1.0.0\x00|g' /home/user/Desktop/myappimg/usr/lib/libQt5Network.so.5
   for future Qt versions, try to detect changes with: strings libQt5Network.so.5 | grep '1\.0\'.
- Create the final AppImage with:
   ./appimagetool-x86_64.AppImage /home/user/Desktop/myappimg

Snap:
1. Use Ubuntu 16.04 or the latest supported LTS by snapcraft
2. Run snapcraft in the stuff/deployment/linux/snap directory after reviewing the yaml file
3. Upload new snap to snap store

    