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
KML    | Generic track | KML with <gx:track> elements             | yes    | -
KML    | Sky Dolly     | Sky Dolly KML                            | -      | yes

## Import

## Export
In order to export a flight:

- In the *logbook* (key **F1**), load the desired *flight*
- Choose the desired export format via menu entry *File \| Export*

An export dialog will appear, with common and plugin specific export options.

### Common Export Options

#### Formation Flight

#### Sample Rate
Resampling is the process of calculating data points at a different frequency (in Hertz) than the original sample rate (also in Hertz). This involves interpolation of the original sampled values, to get another "smooth interpolation curve" through the new sample points. Various interpolation methods exist, such as linear and cubic interpolation.

Sky Dolly applies cubic spline interpolation (Hermite splines) for flight position, velocity and acceleration and mostly linear interpolation for instrument lever positions and flaps positions, for instance.

There are different scenarios why resampling is applied. The two major cases are upsampling (the data is evaluated at a *higher* frequency than the original sample rate) and downsampling (the data is evaluated at a *lower* frequency than the original sample rate):

- Upsampling is done if the recorded data has been sampled only every second (1 Hz), but we need the data for every simulated frame (essentially in the same range as the displayed frames per second, but possibly even higher). So the data is resampled with, say, 60 Hz.
- Downsampling can be done in order to reduce the amount of data. For instance if an aircraft was recorded in the flight simulator for every *simulated frame* (typically in the same range as the displayed frames per second, possibly even higher), but for data analysis reasons it may be sufficient to have the aircraft position sampled only every second (1 Hz). In essence that means a data reduction by a factor of 60 in this example.

### Comma-Separated Values (CSV)
The CSV format is a simple text format, containing an optional header with column names followed by values which are typically separated by a comma. There are however variants which use other *delimiter characters*, such as tab stops or semicolons.

[Index](index.md) \| [Import and Export](import-export.md)
