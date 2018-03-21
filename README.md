![Logo](https://raw.githubusercontent.com/joshirio/iota-cooler/master/resources/icons/iotacooler_64.png "logo")
# IOTAcooler
A simple and cross-platform cold wallet for IOTA. At its core, IOTAcooler uses standard IOTA multisig transfers to split the signing process over two devices, one online (hot signer) and the other offline (cold signer). In this way, the offline signing seed is never exposed to the internet, which is the major attack vector. For best security, consider running IOTAcooler on a live linux distro while cold signing a transaction.

The multisig signing is handled by [smidgen](https://github.com/bitfinexcom/smidgen), a nodejs command line multisig utility based on thr IOTA javascript API, created by Bitfinex.

### Features
- Wallet encryption (AES-256) with password. Wallet does not store the offline signing key.
- Multiple receiving addresses per transaction (batching)
- Address reuse detection when sending to addresses with outgoing transfers
- Two step transaction signing, online and offline
- Built-in promote and reattach utility

### Limitations
- Only one receiving address a time. A new address is generated automatically on transfers.
- Requires to always backup the wallet file after each transfer. Store and sync it in the cloud to be safe (wallet is encrypted and doesn't hold the offline signing key).
- At this time. restoring funds from only the multisig seeds without the wallet file is tricky and only possible with smidgen, the command line utility for multisig transfers.

### How it looks
![Screenshot](https://raw.githubusercontent.com/joshirio/iota-cooler/master/stuff/screenshots/mainwindow.png "Wallet screenshot")

### Demo video
[Click here for a short YouTube demo](https://youtu.be/MegEEOyEkgk)

### Download
IOTAcooler works on Windows, macOS and Linux. Visit the [releases page](https://github.com/joshirio/iota-cooler/releases) and download the appropriate executable/installer for your OS.

### Building from source
Unpack archive and compile with
```
cd iotacooler
qmake -config release
make
```
To run, [iotacooler-smidgen](https://github.com/joshirio/iota-cooler-smidgen) is required (`/usr/bin/iotacooler-smidgen`). iotacooler-smidgen is a fork of smidgen with added functionality like promoting, address reuse checking on multisig transfers and changes to allow building nodejs binaries with `pkg`.

NOTE: The documentation is still in progress, including the building, running and packaging instructions. The [project wiki](https://github.com/joshirio/iota-cooler/wiki) will include user guides and detailed instructions, including on contributing with code or translations.

### License
MIT, see LICENSE file.

### Support this project
[Donate IOTA](https://github.com/joshirio/iota-cooler/blob/master/doc/donate.md)
