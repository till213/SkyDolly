# Locations

## About
Sky Dolly comes with a set of default locations (also known as "system locations" - also refer to table ENUM_LOCATION_TYPE). Those default locations are imported ("migrated") at application start as needed, based on the CSV file Locations.csv.

## Locations.ods
The _master_ file is the [Locations.ods](Locations.ods), an _Open Document Spreadsheet_ file which can be edited with e.g. LibreOffice Calc and Excel.

The spreadsheet contains a formula that re-generates a UUID each time a new row is added and/or the document is saved. This UUID can be used for newly added location rows and serves as "migration ID".

The first spreadsheet tab contains the locations, the second tab contains the _categories_ which are helpful to choose the proper symbolic ID for the _Category_ column.

## Locations.csv
The [Locations.csv](Locations.csv) file is embedded in the executable (Persistence library) with the Qt resource system and is read at each application start and used as migration source.

The separator is expected to be a comma (,) and strings (like in the _description_ column) are optionally to be put between double quotes ("), especially if they contain commas themselves. Likewise floating point numbers must use the decimal point (.).

### Export With LibreOffice
Exporting the CSV with LibreOffice Calc is straight-forward, as LibreOffice Calc supports embedding strings in double quotes and supports several options for the separating character, including the comma (,).

- Choose _Save as..._
- Choose _CSV_ as format, with the following options:
  * Choose the comma as separator
  * Embed strings in double quotes
  
### Export With Excel
Excel essentially determines the floating number format and as a consequence also the delimiter character based on the current system locale. As e.g. for the German locale the floating point number format uses the comma (,) as decimal "point" the CSV delimiter character is typically chosen to be the semicolon (;) instead of the comma (,).

In order to export the CSV with the desired commma (,) as separator the locale may have to be changed to "English" in the operating system settings. Once the locale is set accordingly (and Excel is restarted) the locations may be saved as CSV as follows:

#### Excel 2016
- Choose _Save as..._
- Choose _CSV UTF-8 (Comma delimited) (.csv)_ as format

#### Excel 2019
- Choose _Save as..._
- Choose _CSV UTF-8 (Comma delimited) (*.csv)_ as format
