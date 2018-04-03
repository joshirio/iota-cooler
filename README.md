![Logo](https://raw.githubusercontent.com/joshirio/iota-cooler/master/resources/icons/iotacooler_64.png "logo")
# IOTAcooler
A simple and cross-platform cold wallet for IOTA. At its core, IOTAcooler uses standard IOTA multisig transfers to split the signing process over two devices, one online (hot signer) and the other offline (cold signer). In this way, the offline signing seed is never exposed to the internet, which is the major attack vector. For best security, consider running IOTAcooler on a live linux distro while cold signing a transaction.

The multisig signing is handled by [smidgen](https://github.com/bitfinexcom/smidgen), a nodejs command line multisig utility based on the IOTA javascript API, created by Bitfinex.

### Features
- Wallet encryption (AES-256) with password. Wallet doesn't store the offline signing key.
- Multiple receiving addresses per transaction (batching)
- Address reuse detection when sending to addresses with outgoing transfers
- Two step transaction signing, online and offline
- Built-in promote and reattach utility

### Limitations
- Only one receiving address a time. A new address is generated automatically on transfers.
- Requires to always backup the wallet file after each transfer. Store and sync it in the cloud to be safe (wallet is encrypted and doesn't hold the offline signing key).
- At this time, restoring funds from only the multisig seeds without the wallet file is tricky and only possible with smidgen, the command line utility for multisig transfers. **(development fix in progress)**

### How it looks
![Screenshot](https://raw.githubusercontent.com/joshirio/iota-cooler/master/stuff/screenshots/mainwindow.png "Wallet screenshot")

### Demo video
[Click here for a short YouTube demo](https://youtu.be/MegEEOyEkgk)

### Download
IOTAcooler works on Windows, macOS and Linux. Visit the [releases page](https://github.com/joshirio/iota-cooler/releases) and download the appropriate executable/installer for your OS. For Arch Linux based distributions, there's also an [AUR package](https://aur.archlinux.org/packages/iotacooler/) for building from source, this is also the most secure option, since you don't have to trust the precompiled binaries.

### Building from source
Clone the source repository and compile with
```
git clone --recurse-submodules https://github.com/joshirio/iota-cooler.git
cd iota-cooler
qmake -config release
make
```
To run, [iotacooler-smidgen](https://github.com/joshirio/iota-cooler-smidgen) is required (`/usr/bin/iotacooler-smidgen`). iotacooler-smidgen is a fork of smidgen with added functionality like promoting, address reuse checking on multisig transfers and changes to allow building nodejs binaries with `pkg`.
See 
[deployment](https://github.com/joshirio/iota-cooler/blob/master/stuff/deployment/README.md) 
for building and packaging steps.
Please also check out the [PKGBUILD script](https://github.com/joshirio/iota-cooler/blob/master/stuff/deployment/linux/PKGBUILD) as an example.

NOTE: The documentation is still a work in progress. The [project wiki](https://github.com/joshirio/iota-cooler/wiki) will include user guides and detailed instructions, including on contributing with code or translations.

### License
MIT, see LICENSE file.

### Support this project
[Donate IOTA](https://github.com/joshirio/iota-cooler/blob/master/doc/donate.md)
