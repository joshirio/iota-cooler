CHANGELOG

Version x.x
===========

2018-04-04

### New Features
- Recover wallet balance from seeds (while keeping seeds offline), issue #2
- Clipboard guard to warn about possible unwanted changes (malware) to copied addresses
- Open wallet files by drag and drop on the start window
- Add custom tags to transactions, issue #4
- Option to provide custom seeds for wallet creation, issue #1
- Show unconfirmed balance for incoming transactions
- Show notifications for incoming transactions
- Show full transfer history (very fast), issue #7
- Debian package for Ubuntu based distros

### Improvements
- Address reuse detection after snapshots
- Reduce wallet size and bloat over time by pruning redundant smidgen transactions
- Detect and warn if an old wallet version is opened by checking if currrent address was spent
- Show balance in IOTA SI units (Pi, Ti, Gi, Mi, Ki, i) where useful

### Bug Fixes
- Show correct address on wallet creation (Windows)
- Correctly enforce device roles

Version 1.0
===========

2018-03-21

- Initial release
