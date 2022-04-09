# SHA-256 Checksums

Checksums (hash values, digests) allow to verify the integrity of a given file, such as a ZIP archive. If both the newly computed and published (e.g. in this SHASUM256.md file), well-known and trusted checksums match then we can conclude for instance that:

- The download succeeded
- No one changed the content of the file(s)

If on the other hand the checksums differ in only a single digit then we can conclude that:

- A download error occured
- The downloaded file is not the same as the one we are expecting from the original source (https://github.com/till213/SkyDolly/releases)

There are differnt checksum algorithms. One of the most widely used set of hash functions is the [SHA-2](https://en.wikipedia.org/wiki/SHA-2), of which SHA-256 is part of. The number of bits of the generated checksum (digest) is 256, hence its name.

Note that a succesful comparison of a computed checksum against a published checksum merely states that the files are identical, and that the copy has not been tampered with by someone else. The files themselves are not "encrypted" by the checksum, nor does the checksum itself say anything about the de facto origin (website) of the downloaded copy (it may have been downloaded from another URL than from https://github.com/till213/SkyDolly/releases).

## How To Compute The SHA-256 Checksum on Windows

There are various methods for computing SHA checksums on Windows 10. One fairly straight-forward method which involves the Windows PowerShell is described in the following steps.

- Open the Windows PowerShell
- Navigate to the download folder
- Execute _Get-FileHash_ [path to file]

Example:

```
PS C:\path\to\download\folder> Get-FileHash .\SkyDolly-v0.10.0.zip
```

Once the checksum has been calculated compare it with the corresponding checksum published below. If you have a mismatch (e.g. due to a download error) then your copy is not identical to the one originating from here: https://github.com/till213/SkyDolly/releases (note that copies dowloaded from https://flightsim.to/file/9067/sky-dolly are expected to be identical, too). In such a case retry with the download and remove (don't use) the non-matching copy from your computer.

## Checksums

### v0.10

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 90D92D24BD44DD23CBC7E0D3CFE5962C7E5182FE970F619F89E6AD3B396C92CA | SkyDolly-v0.10.0.zip

### v0.9

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 7483CF42C012CF59D703D648CB59EE1060C916177CDB4D3816A58D98A1E702D5 | SkyDolly-v0.9.0.zip

### v0.8

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 6B30F4FB53F57338CD85D6717FEDBC7280194D610FAE58ED011B2573D99027CB | SkyDolly-v0.8.6.zip
SHA256    | 942338E85710D5012D8A2680C5D624C4FDD49FF5974AABC6F5C77CE7C9666A73 | SkyDolly-v0.8.5a.zip
SHA256    | E1F214E83FD0198CAB662D283B3E7D2E2D71BB97B42195DF96785F90D8DE98C3 | SkyDolly-v0.8.4.zip
SHA256    | A36FD4702D7DEB02549A826299E6E18F2CCF75618B170FDFC6BE3EAC0C9B7BE1 | SkyDolly-v0.8.3.zip
SHA256    | 37541E550B939C7746913FEE991EA1CF05F526CE3EA269F10841B100779535DB | SkyDolly-v0.8.2.zip
SHA256    | 958E3F6A447ADD5383FB6A6830A126C6504E27C987A40A1C6C73236B8E045899 | SkyDolly-v0.8.1.zip
SHA256    | 199CD3B28BB31F04B1A4F12543516366FE49D10842E49AC6285CC967DF3441A6 | SkyDolly-v0.8.0.zip

### v0.7

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 22523889D1A0C01A3C8AF0155C7174CD8C7B4FA8712DBF82C2CC49006DFE457D | SkyDolly-v0.7.1.zip
SHA256    | 9C90E8D1885F08118254E4BD06355C06B0E980069573D5147A23E5F7CDEC1A82 | SkyDolly-v0.7.0.zip

### v0.6

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | 97AE2737620EFE9F941559B9EEBF2D1B2C3B4206DD1165ABF532F9796273A773 | SkyDolly-v0.6.3.zip
SHA256    | 2D36FBD01761E686067204892B54346CBF0870F3105CB7481E69E713CD3215E0 | SkyDolly-v0.6.2.zip
SHA256    | BCEBB7E8E6D645D64C59AC49B6D1D2A321107754FFE912BDF95E1949BAF6E326 | SkyDolly-v0.6.1.zip
SHA256    | 68B3FC72E081C4B52FF89D4AD9ABB005EFB313DE14A864E07493779953343FD8 | SkyDolly-v0.6.0.zip

### Older Releases

Algorithm | Hash                                                             | File
----------|------------------------------------------------------------------|---------------------
SHA256    | CC61F9B8CF748E5A07188527579417F7FC9E949CB6B3D839F759D550FF871647 | SkyDolly-v0.5.2.zip
SHA256    | 3E5843394732050E9E85A4AF6AB4DB67FC715758B7CCA377A33093539552671F | SkyDolly-v0.5.1.zip
SHA256    | C773E56489D598F1CA70EA7EBB002CCF8F34A6B55BB1E6C4A23C132E4E19814E | SkyDolly-v0.5.0.zip
SHA256    | 91CF4C974549DED0433200A295823715DA673C902FFCF6FDC38C9A8191F9E616 | SkyDolly-v0.4.2.zip
SHA256    | 6ABCE8AB962FDCBF4DD30246D52B39D96B3C4986BD19BD8312606567BB592941 | SkyDolly-v0.4.1.zip
SHA256    | 198DC1D9D92AA1E00D1F59A0F207090358D6574D408124CFF16C6AA0A0D706FB | SkyDolly-v0.4.0.zip
SHA256    | B716E1AACAA2B873ABE3F3D3FE022B8CE3ABF49A2E855BEF0EFE8FCA75B19EF4 | SkyDolly-v0.3.1.zip
SHA256    | AD4E794BF68541A968A8F4AAF63054EADC7FDAB321C85A6081C8E6D539C5D323 | SkyDolly-v0.3.0.zip
SHA256    | 93C9E2705E2220A7E2EDA2429281B77795C4E2E7E0ABD2CE5D9E2EFF84467AD3 | SkyDolly-v0.2.1.zip
SHA256    | 6F1F28DF105664A55692CD9E3D864A486CF4BBAD4333F9A583D2A24682D486D8 | SkyDolly-v0.2.0.zip
SHA256    | ED9DA8FFAC77B1CE21B93E1CA3ACE0F1EAA2CCD2D3F5CBB2359FB2AD072547E6 | SkyDolly-v0.1.0.zip
