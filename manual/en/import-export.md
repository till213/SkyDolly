# Import and Export

[Index](index.md) \| [Import and Export](import-export.md)

Sky Dolly supports a range of file formats for import and export of *flight* or *aircraft* data into (from) Sky Dolly. The import and export modules are implemented as *plugins*, that is they are loaded on demand into memory and unloaded after use.

## Supported Formats
The following file formats are supported:

Format | Flavour       | Description                              | Import | Export
-------|---------------|------------------------------------------|--------|-------
CSV    | Sky Dolly     | Sky Dolly comma-separated values         | yes    | yes
CSV    | flightradar24 | flightradar24.com comma-separated values | yes    | yes
GPX    | -             | GPS exchange format                      | yes    | yes
IGC    | -             | International gliding commission format  | yes    | yes
JSON   | GeoJSON       | GeoJSON                                  | -      | yes
KML    | flightaware   | flightaware.com KML                      | yes    | -
KML    | flightradar24 | flightradar24.com KML                    | yes    | -
KML    | Sky Dolly     | Sky Dolly KML                            | -      | yes

[Index](index.md) \| [Import and Export](import-export.md)
