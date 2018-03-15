# Wallet File Format

1. magic number 0x494f5441434f4f4c (IOTACOOL) (8 bytes)
2. wallet file version (quint32)
3. SHA-1 data checksum ASCII string of base 64 encoded aes_wallet_data
4. AES-256 IV-Vector (CBC) random ASCII string of length 16
5. AES-256 encrypted JSON wallet data (variable length) encoded as base64

##### Example:

```
IOTACOOL
version:1
data_checksum:a94a8fe5ccb19ba61c4c0873d391e987982fbbd3
aes_iv:randomstring1234
aes_wallet_data:dGVzdGRhdGExMjM0NTY3ODk=
```

---

# Wallet JSON

The decrypted JSON contains the online seed (2nd multisig party), current receiving address,
past used addresses, a backup of a clean state smidgen wallet file (ie. no pending txs), which
is restored when a pending multisig tx is cancelled, current smidgen wallet file with pending txs 
that must be signed offline (if any), current wallet operation (enum) and current op arguments,
list of past spending transactions (signed outgoing multisig txs).

##### Example:

```
{
  "onlineSeed": "9MSXOVRXE9XFOIHLXBTOWEOJCZKIQRJMEPPBCLJWVYHFEPHZQQDHZVGEMQGYFWSBIO9YDJMYRMEIXEAHC",
  "currentColdWallet": "c21pZGdlbmNvbGR3YWxsZXRtdWx0aXNpZ2ZpbGV3aXRocGVuZGluZ3R4c2lmYW55",
  "cleanColdWalletBackup": "aGVyZXRoZWNvbGR3YWxsZXRmcm9tc21pZGdlbmluYWNsZWFuc3R",
  "currentOperation": 1,
  "currentOpArgs": [
    "arg1",
    "arg2",
    "arg3"
  ],
  "currentAddress": "TYWQMBTHFVMVMAKVOAVHWA9GIENPZPVQZKYDROHDVIYKEZMXQAQWLHADBEIIBPFNPYUBMHFL9DGPWWLBWUB9CXMCFC",
  "pastUsedAdresses": [
    "NVYUGAQTT9AQJZZ9UKVKFCNTAZUASGCKARFMCZIDFVBASGNTAGSEPRL9CTJLWUVENA9UQWY9UVMEPFLLWNPEXDL9PD",
    "NVYUGAQTT9AQJZZ9UKVKFCNTAZUASGCKARFMCZIDFVBASGNTAGSEPRL9CTJLWUVENA9UQWY9UVMEPFLLWNPEXDL9PD"
  ],
  "pastSpendingTransactions": [
    {
      "tailTxHash": "SUGDOXURVKGCNCBRRLEXKGTBRRUHF9FHVDZGJOBHACMMIZM9YKSTBPDUMHMFHY9POVKDNRXBFWYZ99999",
      "amount": "123456i",
      "spendingAddress": "ENSOEVANAVZTHUTIT9DWRCPLZDNUQKSBICPHNNYYGSNZX9TWZPRDKXCLJVWZYABPDBXYAUQEZLDZUENDX",
      "receivingAdresses": [
        "NUULVNNSPJXH9DYYPFSVCR9BOWKXLOEGHBUREDABZ9MZMHWLAFMHNFUNWLKMMPDIYHKCQZQVKKLTXNZPF",
        "",
        ""
      ],
      "tag": ""
    },
    {
      "tailTxHash": "SUGDOXURVKGCNCBRRLEXKGTBRRUHF9FHVDZGJOBHACMMIZM9YKSTBPDUMHMFHY9POVKDNRXBFWYZ99999",
      "amount": "123456i",
      "spendingAddress": "ENSOEVANAVZTHUTIT9DWRCPLZDNUQKSBICPHNNYYGSNZX9TWZPRDKXCLJVWZYABPDBXYAUQEZLDZUENDX",
      "receivingAdresses": [
        "NUULVNNSPJXH9DYYPFSVCR9BOWKXLOEGHBUREDABZ9MZMHWLAFMHNFUNWLKMMPDIYHKCQZQVKKLTXNZPF",
        "",
        ""
      ],
      "tag": ""
    }
  ]
}
```
