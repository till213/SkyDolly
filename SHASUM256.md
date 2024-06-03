# Sky Dolly SHA-256 Checksums

Checksums (hash values, digests) allow to verify the integrity of a given file, such as a ZIP archive. If both the newly computed and published, well-known and trusted checksums match then we can conclude that:

- The download succeeded
- No one changed the content of the file(s)

## How To Compute The SHA-256 Checksum on Windows

There are various methods for computing [SHA-2](https://en.wikipedia.org/wiki/SHA-2) checksums on Windows 10 (and other platforms). One fairly straight-forward method which involves the Windows PowerShell is as follows:

- Open the Windows PowerShell
- Navigate to the download folder
- Execute _Get-FileHash_ [path to file]

Example:

```
PS C:\path\to\download\folder> Get-FileHash .\SkyDolly-v0.17.2.zip
```

Once the checksum has been calculated compare it with the corresponding checksum published below. If you have the slightest mismatch:

- Don't panic! It could be because of a simple download error - try again
- If the mismatch persists: don't use your copy! It is not identical to the one originally released on either [GitHub.com](https://github.com/till213/SkyDolly/releases) or [flightsim.to](https://flightsim.to/file/9067/sky-dolly).

## Checksums

### v0.17

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | b3c31e9e2839bd7ccce6a532ed5270d3b5c6a91dbba18eb828c78d97db003e2b | SkyDolly-v0.17.3.zip
SHA256    | d065801b914770f515e7ac327430f9dffb0e4c01c3d9917663ff7c9ab65067b8 | SkyDolly-v0.17.2.zip
SHA256    | e077d8c5f1c9e1db507471f74b8002d2eda0d87e6ae69c7cd57c90200e5e4fae | SkyDolly-v0.17.1.zip
SHA256    | 2e4d8f65dfcb83017b9f562157e6376ee6179e67adee3f2c5f066fbf799bec93 | SkyDolly-v0.17.0.zip

### v0.16

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | acf5d3b6cbe89568f472cbac0a5bfd56768f94fd069a9f7221a2c3b1727d99d3 | SkyDolly-v0.16.3.zip
SHA256    | 34b29c7246d18f828286df471446493e66c9bf9f5e300bc068c7571f3328050f | SkyDolly-v0.16.2.zip
SHA256    | 15bb59f017401f2aeb5c2b52e6310df37e7caf1ee0c7563d8e379c86da379586 | SkyDolly-v0.16.1.zip
SHA256    | 96369a973073ba235194597612a123c972fa2dd39c475e5e444a226cecd39ee5 | SkyDolly-v0.16.0.zip

### v0.15

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 3d29f3e93828991e3642bc23db489f0c5c1e0cef4d74417916fa3563b31739cb | SkyDolly-v0.15.1.zip
SHA256    | 3653fce830e57487e38dcc7886a97463bca4407cd30d38aa27401aed01f1f4a0 | SkyDolly-v0.15.0.zip

### v0.14

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | b2095961d77aaa9ae284199dd138a6af8fd60a3580a422a25d3f5a43bad76bd9 | SkyDolly-v0.14.0.zip

### v0.13

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 4c04aa0ad081d38a7c3cc042109d341b76cc92c165fa312cd8bcd832fccb6bfb | SkyDolly-v0.13.1.zip
SHA256    | aa990d71b16c06cce7a79c0310bf497b5cd3339c919bf6eb70ad2873665442bd | SkyDolly-v0.13.0.zip

### v0.12

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 9bf59ebcd021bb16130f1caabd90c78ee5f854f49f20b57bcbe4917440ac19aa | SkyDolly-v0.12.0.zip

### v0.11

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 73463cb7d78c01f7bde7ab99113e6693841482741ad96a51c19a846308a79758 | SkyDolly-v0.11.1.zip
SHA256    | 083f5eb20c33fd80dc30bd3678d191a38d81b72908dc1921813e1d1c3c06fd7c | SkyDolly-v0.11.0.zip

### v0.10

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 90d92d24bd44dd23cbc7e0d3cfe5962c7e5182fe970f619f89e6ad3b396c92ca | SkyDolly-v0.10.0.zip

### v0.9

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 7483cf42c012cf59d703d648cb59ee1060c916177cdb4d3816a58d98a1e702d5 | SkyDolly-v0.9.0.zip

### v0.8

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 6b30f4fb53f57338cd85d6717fedbc7280194d610fae58ed011b2573d99027cb | SkyDolly-v0.8.6.zip
SHA256    | 942338e85710d5012d8a2680c5d624c4fdd49ff5974aabc6f5c77ce7c9666a73 | SkyDolly-v0.8.5a.zip
SHA256    | e1f214e83fd0198cab662d283b3e7d2e2d71bb97b42195df96785f90d8de98c3 | SkyDolly-v0.8.4.zip
SHA256    | a36fd4702d7deb02549a826299e6e18f2ccf75618b170fdfc6be3eac0c9b7be1 | SkyDolly-v0.8.3.zip
SHA256    | 37541e550b939c7746913fee991ea1cf05f526ce3ea269f10841b100779535db | SkyDolly-v0.8.2.zip
SHA256    | 958e3f6a447add5383fb6a6830a126c6504e27c987a40a1c6c73236b8e045899 | SkyDolly-v0.8.1.zip
SHA256    | 199cd3b28bb31f04b1a4f12543516366fe49d10842e49ac6285cc967df3441a6 | SkyDolly-v0.8.0.zip

### v0.7

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 22523889d1a0c01a3c8af0155c7174cd8c7b4fa8712dbf82c2cc49006dfe457d | SkyDolly-v0.7.1.zip
SHA256    | 9c90e8d1885f08118254e4bd06355c06b0e980069573d5147a23e5f7cdec1a82 | SkyDolly-v0.7.0.zip

### v0.6

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 97ae2737620efe9f941559b9eebf2d1b2c3b4206dd1165abf532f9796273a773 | SkyDolly-v0.6.3.zip
SHA256    | 2d36fbd01761e686067204892b54346cbf0870f3105cb7481e69e713cd3215e0 | SkyDolly-v0.6.2.zip
SHA256    | bcebb7e8e6d645d64c59ac49b6d1d2a321107754ffe912bdf95e1949baf6e326 | SkyDolly-v0.6.1.zip
SHA256    | 68b3fc72e081c4b52ff89d4ad9abb005efb313de14a864e07493779953343fd8 | SkyDolly-v0.6.0.zip

### Older Releases

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | cc61f9b8cf748e5a07188527579417f7fc9e949cb6b3d839f759d550ff871647 | SkyDolly-v0.5.2.zip
SHA256    | 3e5843394732050e9e85a4af6ab4db67fc715758b7cca377a33093539552671f | SkyDolly-v0.5.1.zip
SHA256    | c773e56489d598f1ca70ea7ebb002ccf8f34a6b55bb1e6c4a23c132e4e19814e | SkyDolly-v0.5.0.zip
SHA256    | 91cf4c974549ded0433200a295823715da673c902ffcf6fdc38c9a8191f9e616 | SkyDolly-v0.4.2.zip
SHA256    | 6abce8ab962fdcbf4dd30246d52b39d96b3c4986bd19bd8312606567bb592941 | SkyDolly-v0.4.1.zip
SHA256    | 198dc1d9d92aa1e00d1f59a0f207090358d6574d408124cff16c6aa0a0d706fb | SkyDolly-v0.4.0.zip
SHA256    | b716e1aacaa2b873abe3f3d3fe022b8ce3abf49a2e855bef0efe8fca75b19ef4 | SkyDolly-v0.3.1.zip
SHA256    | ad4e794bf68541a968a8f4aaf63054eadc7fdab321c85a6081c8e6d539c5d323 | SkyDolly-v0.3.0.zip
SHA256    | 93c9e2705e2220a7e2eda2429281b77795c4e2e7e0abd2ce5d9e2eff84467ad3 | SkyDolly-v0.2.1.zip
SHA256    | 6f1f28df105664a55692cd9e3d864a486cf4bbad4333f9a583d2a24682d486d8 | SkyDolly-v0.2.0.zip
SHA256    | ed9da8ffac77b1ce21b93e1ca3ace0f1eaa2ccd2d3f5cbb2359fb2ad072547e6 | SkyDolly-v0.1.0.zip
