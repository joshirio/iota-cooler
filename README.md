![Logo](https://raw.githubusercontent.com/joshirio/iota-cooler/master/resources/icons/iotacooler_64.png "logo")
# IOTAcooler
A cross-platform cold storage and spending wallet for IOTA. At its core, IOTAcooler uses standard IOTA multisig addresses to split the signing process over two devices, one online (hot signer) and the other offline (cold signer). In this way, the offline signing seed is never exposed to the internet, securing your funds against popular attack vectors exploited by malware. For best security, consider running IOTAcooler from a live linux image while cold signing a transaction. Please check out our [user guide](https://github.com/joshirio/iota-cooler/wiki/User-Guide) for additional security considerations and a quick software tour.

IOTAcooler is designed with security and speed in mind. All sensitive steps are done completely offline, including the wallet recovery feature. IOTAcooler is really fast because the wallet is stateful, meaning that all past transactions, addresses and other states are saved into the wallet file, also because it uses light IOTA API requests for checking current transfers on the Tangle, which is very practical in case numerous or big transactions are part of the wallet history. The downside of this is that some specific information for incoming transactions is only visible on a tangle explorer.

The multisig signing is handled by [iotacooler-smidgen](https://github.com/joshirio/iota-cooler-smidgen), based on [smidgen](https://github.com/bitfinexcom/smidgen). Smidgen is a nodejs command line multisig utility using the official IOTA javascript library and created by Bitfinex.

### Features
- Wallet encryption (AES-256) with password. Wallet file never stores the offline signing key.
- Multiple receiving addresses per transaction (batching)
- Address reuse detection when sending to addresses with outgoing transfers, even after snapshots
- Snapshot proof [wallet recovery](https://github.com/joshirio/iota-cooler/wiki/Wallet-Recovery-from-IOTAcooler-Seeds) from seeds with offline signing process (recovery seeds remain safe)
- Two step transaction signing, online and offline
- Clipboard guard to warn about possible unwanted changes (malware) to copied addresses
- Support for [custom generated seeds](https://github.com/joshirio/iota-cooler/wiki/Generating-IOTA-Seeds-Securely)
- Built-in promote and reattach utility
- Automatic promoting and reattaching thanks to [CarrIOTA Field](http://field.carriota.com/)
- Fast transaction history retrieval, even with big bundles (CarrIOTA Field donations :)
- Remote proof-of-work: fast transactions and less power usage
- Detect and warn if an old wallet version is opened

### Limitations
- Only one receiving address a time. A new address is generated automatically on transfers. You can use traditional spending wallets with small amounts which then are periodically refilled from secure cold storage wallets.
- Requires to always backup the wallet file after each transfer. Store and sync it in the cloud to be safe (wallet is encrypted and doesn't hold the offline signing key). If an old version is opened by mistake, IOTAcooler will detect this and show a warning.
- Some information, like source addresses for incoming transfers, is only visible with a tangle explorer due to light API requests usage.

### How it looks
![Screenshot](https://raw.githubusercontent.com/joshirio/iota-cooler/master/stuff/screenshots/mainwindow.gif "Wallet screenshot")

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
To run, [iotacooler-smidgen](https://github.com/joshirio/iota-cooler-smidgen) is required. iotacooler-smidgen is a fork of smidgen with added functionality like promoting, address reuse checking on multisig transfers, custom recovery commands and changes to allow building nodejs binaries with `pkg`.
See [deployment](https://github.com/joshirio/iota-cooler/blob/master/stuff/deployment/README.md) for building and packaging steps.
Please also check out the [PKGBUILD script](https://github.com/joshirio/iota-cooler/blob/master/stuff/deployment/linux/PKGBUILD) as an example.

### User Guide
The [project wiki](https://github.com/joshirio/iota-cooler/wiki) includes an user guide and other useful information.

### License
MIT, see [LICENSE](https://github.com/joshirio/iota-cooler/blob/master/LICENSE) file.

### Support this project
[Donate IOTA](https://github.com/joshirio/iota-cooler/blob/master/doc/donate.md)
