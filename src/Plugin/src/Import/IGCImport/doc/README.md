# IGC (International Gliding Commission) File Format

## About

The IGC format is (mostly) created by flight recorders in gliders.

## Record Types

```
A - FR manufacturer and FR serial no.
B - Fix
C - Task/declaration
D - Differential GNSS
E - Event
F - Satellite constellation
G - Security
H - File header
I - List of additional information after the basic B-record data
J - List of additional information after the basic K-record data
K - Frequent data, additional to the B-record
L - Logbook/comments
M, N, etc. - Spare
```

Generally, the B, I, J, L and K records have a maximum of 99 characters, excluding CRLF which is hidden and does not appear in text 
form. Some Record types are on only one line, others can be on several lines.

The A-Record is always the first in the file, and the last that is relevant to the data recorded in flight is the Security (G) Record 
that allows the validity of fix data to be checked at any time through the IGC VALI system.

After the single-line A record is the multi-line Header (H) Record, followed by the I and J Records that identify data included in B 
and K record lines later in the file. These are followed by other Record types including the task declaration (C) Record,and the Satellite 
Constellation (F) used for the fixes (B) that follow.

Time-specific Record lines are placed in the file in time order using either GNSS fix-time if the GNSS is locked on, otherwise time is 
taken from the recorder’s Real Time Clock (RTC); these are B (fix), E (event), F (constellation change) & K (additional data).

The logbook/comments (L) Record data may be placed anywhere after the H, I and J Records and there may be several L-record lines in the file.

### Mandatory Records

```
A, H, I, B, F, G
```

### Typical Order

```
A - FR manufacturer and identification (always first)
H - File header (always after the A-record)
I - List of other data added at end of each B record
J - List of other data added at end of each K record
C - Task/declaration (if used)
L - Logbook/comments (if used)
D - Differential GNSS (if used)
F - Initial Satellite Constellation
B - Fix plus any additional data listed in I Record
B - Fix plus any additional data listed in I Record
E - Pilot Event (PEV)
B - Fix plus any additional data listed in I Record
K - Additional data as listed in J Record
B - Fix plus any additional data listed in I Record
B - Fix plus any additional data listed in I Record
F - Constellation change
B - Fix plus any additional data listed in I Record
K - Additional data as defined in J Record
B - Fix plus any additional data listed in I Record
E - Pilot Event (PEV)
B - Fix plus any additional data listed in I Record
B - Fix plus any additional data listed in I Record
B - Fix plus any additional data listed in I Record
K - Additional data as defined in J Record
L - Logbook/comments (if used)
G - Security record
L - Data added after the flight is completed
```

## Units

| What                                            | Unit                                                                                   |
|-------------------------------------------------|----------------------------------------------------------------------------------------|
| Date (of the first line in the B record)        | DDMMYY (day, month, year, with respect to Universal Time Co-ordinated (UTC)            |
| Time                                            | HHMMSS (UTC)                                                                           |
| Latitude and Longitude                          | Degrees, minutes and decimal minutes to three decimal places, with N,S,E,W designators |
| Altitude                                        | Metres (*)                                                                             |
| Direction                                       | Degrees True, clockwise from 000 (North)                                               |
| Distance                                        | Kilometres and decimal kilometres                                                      |
| Speed                                           | Kilometres per hour                                                                    |

(*) separate records for (1) GNSS altitude above the WGS84 ellipsoid, and (2) ICAO ISA pressure altitudes.

### Format

**Date** - DDMMYY (with leading 0)

**Time** - HHMMSS (UTC)

**Lat/Long** - D D M M m m m N D D D M M m m m E

#### Details
DD - Latitude degrees with leading 0 where necessary
DDD - Longitude degrees with leading 0 or 00 where necessary
MMmmmNSEW - Lat/Long minutes with leading 0 where necessary, 3 decimal places of minutes are mandatory,
followed by N, S, E or W as appropriate

**Altitude** - AAAAAaaa

#### Details
AAAAA - fixed to 5 digits of metres above the WGS84 ellipsoid with leading zero
aaa - where used, the number of altitude decimals (the number of fields recorded are those available for altitude in the
Record concerned, less fields already used for AAAAA)

**Direction** (calculated from successive fixes) - DDDddd

### Details
DDD - fixed to 3 digits with leading 0 where necessary
ddd - where used, the number of direction decimals (the number of fields recorded are those available for direction
in the Record concerned, less fields already used for DDD)

**Distance** (calculated from successive fixes) - DDDDddd

### Details
Kilometres up to 9999 with leading zeros as required and then three decimal places (the last figure will therefore be metres)

**Speed** (calculated from successive fixes) - SSSsss

### Details
SSS - fixed to 3 digits with leading 0
sss - number of speed decimals

## Record Details

### H Record (Header)

Required records:

```
H F DTE DATE: DD MM YY, NN CRLF
H F PLT PILOT IN CHARGE: TEXT STRING CRLF
H F CM2 CREW 2: TEXT STRING CRLF
H F GTY GLIDER TYPE: TEXT STRING CRLF
H F GID GLIDER ID: TEXT STRING CRLF
H F DTM GPS DATUM: WGS84 CRLF
H F RFW FIRMWARE VERSION: TEXT STRING CRLF
H F RHW HARDWARE VERSION: TEXT STRING CRLF
H F FTY FR TYPE: MANUFACTURERS NAME,FR MODEL NAME CRLF
H F GPS RECEIVER: MANUFACTURERS NAME, MODEL NAME, CHANNELS, MAX ALT CRLF
H F PRS PRESS ALT SENSOR: MANUFACTURERS NAME, MODEL NAME, MAXALT CR LF
H F FRS SECURITY OK or SECURITY SUSPECT : TEXTSTRING CRLF
```

Notes:

- Extra spaces between items are added so that it is easier to see the items; these spaces are not present in actual 
IGC files when viewed in text format
- DATE line: NN is the flight number on the day, prefixed by a comma to separate it from the date group

### I Record (Additions to the Fix (B))

| Description         | Size    | Element | Remarks                                                 |
|---------------------|---------|---------|---------------------------------------------------------|
| Number of additions | 2 bytes | NN      | Valid characters 0-9                                    |
| Start byte number   | 2 bytes | SS      | Valid characters 0-9, start byte on each B-record line  |
| Finish byte number  | 2 bytes | FF      | Valid characters 0-9, finish byte on each B-record line |
| 3-letter Code       | 3 bytes | CCC     | Alphanumeric subject                                    |

The byte count starts from the beginning of the B Record, taking the first B in the line as byte one. 

Example:
```
I 01 36 38 FXA CR LF
```

The above line shows that the three numbers for Fix Accuracy (FXA) are recorded between bytes 36 and 38 on each B-record
line. For a device that also records Satellites In Use (SIU), Environmental Noise Level (ENL) inside the FR, and the external
MOP sensor:
```
I 04 3638 FXA 3940 SIU 4143 ENL 4446 MOP CR LF
```

### B Record (Fix)

Not counting the last CRLF, a B record line includes 35 bytes for its basic data, plus those for extra
characters that are defined in the I Record such as Fix Accuracy (FXA).

The required basic data is: UTC, WGS84latitude, WGS84 longitude, fix validity, pressure altitude , GNSS altitude 
with respect to the WGS84 Ellipsoid.

The format of the basic data, with extra spaces for clarity, is:
B HHMMSS DDMMmmmN DDDMMmmmE V PPPPP GGGGG CR LF

| Description  | Size    | Element     | Remarks                                                                      |
|--------------|---------|-------------|------------------------------------------------------------------------------|
| Time UTC     | 6 bytes | HHMMSS      | Valid characters 0-9                                                         |
| Latitude     | 8 bytes | DDMMmmmN/S  | Valid characters N, S, 0-9                                                   |
| Longitude    | 9 bytes | DDDMMmmmE/W | Valid characters E, W, 0-9                                                   |
| Fix validity | 1 byte  | A or V      | Use A for a 3D fix and V for a 2D fix (no GNSS altitude) or for no GNSS data |
| Press Alt.   | 5 bytes | PPPPP       | Altitude in metres relative to the ICAO ISA 1013.25 HPa datum (*)            |
| GNSS Alt.    | 5 bytes | GGGGG       | Altitude in metres above the WGS84 ellipsoid, valid characters 0-9           |

(*) valid characters 0-9 and negative sign "-". Negative values to have negative sign instead of leading zero

### K Record (Less Frequent Fixes)

The K record is for data that is needed less frequently than fix (B) records. The contents of the K record are listed in the J record. 
The following J Record specifies the information in the K Record in the next line (with extra spaces for clarity):

```
J 08 12 HDT CR LF
K HHMMSS 00090 CR LF
``` 

This K Record shows that the true heading (TLC = HDT) is 090 (East).

## Examples

- [2022-01-31-XYY-EXAMPLE-01.IGC](2022-01-31-XYY-EXAMPLE-01.IGC) - Example taken from the offical [IGC specification](https://www.fai.org/sites/default/files/igc_fr_specification_2020-11-25_with_al6.pdf)
- [2022-01-31-XYY-EXAMPLE-02.IGC](2022-01-31-XYY-EXAMPLE-02.IGC) - A real-world example from [xp-soaring.github.io](https://xp-soaring.github.io/igc_file_format/index.html) 

## Resources

- Official IGC specification (file format in appendix A): https://www.fai.org/sites/default/files/igc_fr_specification_2020-11-25_with_al6.pdf
- IGC file format homepage: https://xp-soaring.github.io/igc_file_format/index.html
- An IGC parser TypeScript implementation: https://github.com/Turbo87/igc-parser/
- Example IGC files: http://www.skypolaris.org/igc-files/
- IGC analysis: http://igc-analysis.com
- Flight database and andalysis: https://skylinescondor.com/
