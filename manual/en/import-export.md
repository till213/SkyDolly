# Import and Export

[Index](index.md) \| [Import and Export](import-export.md)

Sky Dolly supports a range of file formats for import and export of *flight* or *aircraft* data into (from) Sky Dolly.

## Supported Formats
The following file formats are supported:

Format | Flavour       | Description                              | Import | Export
-------|---------------|------------------------------------------|--------|-------
CSV    | Sky Dolly     | Sky Dolly comma-separated values         | yes    | yes
CSV    | flightradar24 | flightradar24.com comma-separated values | yes    | yes
CSV    | Position      | Position and attitude data               | -      | -
GPX    | -             | GPS exchange format                      | yes    | yes
IGC    | -             | International gliding commission format  | yes    | yes
JSON   | GeoJSON       | GeoJSON                                  | -      | yes
KML    | flightaware   | flightaware.com KML                      | yes    | -
KML    | flightradar24 | flightradar24.com KML                    | yes    | -
KML    | Generic track | KML with <gx:track> elements             | yes    | -
KML    | Sky Dolly     | Sky Dolly KML                            | -      | yes

## Formats

### CSV - Comma-Separated Values
The CSV format (comma-separated values) is a simple text format organising the data into columns, containing an optional header with column names followed by values which are typically separated by a comma. There are however variants which use other *delimiter characters*, such as tab stops or semicolons.

### GPX - GPS Exchange Format
The GPX format is an XML-based format for exchanging global positioning system (GPS) data (or generally speaking, position data recorded with any global navigation satellite system (GNSS)). Besides tracks containing the actual recored data the format also supports waypoints and routes, including some metadata.

The GPX format is an XML-based format for global positioning system (GPS) data exchange (or generally speaking, position data recorded with any global navigation satellite system (GNSS)). Besides tracks containing the actual recored data the format also supports waypoints and routes, including some metadata.

- The \<wpt> elements specify individual points of interest (e.g. start and end point, including waypoints along the planned route to visit)
- The \<rte> element contains the planned route to get from one waypoint to another
- The \<trk> element contains the actual recorded GNSS data (e.g. GPS data), the actual track that has been taken and recorded

### IGC - International Gliding Commission
The IGC format has been specified by the International Gliding Commission. It is mostly popular for flight recorders used in gliders. The format is a text-based format and consists of various "records", such as:

- Header (H records): flight date, pilot name(s), used flight recorder hardware, and other flight data
- Tasks (C records): the (planned) waypoints, including take-off and landing
- Fixes (B records): the timestamp, location, pressure and GNSS altitude (plus optional extended data such as environmental noise level)

### JSON - JavaScript Object Notation
The JavaScript Object Notation (JSON) is a text-based format designed for data exchange between applications while still being "human-readable" (to some degree). Like XML it merely specifies the syntax of a document, without specifying the semantics.

The semantic is injected by defining the proper keywords, objects and their hierarchies.

A notable example is the GeoJSON format, which specifies geographic features using the JSON syntax.

#### GeoJSON
The GeoJSON format represents geographical data such as points, lines and polygons, by using the JSON format. The format is maintained by the Internet Engineering Taskforce (IETF) and specified by the RFC 7946.

### KML - Keyhole Markup Language
The Keyhole Markup Language (KML) format is used to specify and display geographic data with "earth browsers". It has become well-known with the advent of Google Earth. The format is maintained by the Open Geospatial Consortium (ogc.org). 

## Import
In order to import a flight:

- Choose the desired import format via the *File \| Import* menu entry

An import dialog will appear, with common and plugin specific import options.

### Common Import Options

## CSV - Comma-Separated Values
The *Format* ooption specifies the column layout of the data.

- **FlightRadar24** - the data is in flightradar24.com column layout, essentially position, heading and velocity
- **Sky Dolly** - the data is in Sky Dolly column layout, that is the entire data for the given *aircraft*

Note that while the Sky Dolly format contains almost all recorded *aircraft* data (position, engine, primary flight controls, ....) it does neither contain information about the actual aircraft type nor about the initial flight conditions (simulation time, temperature, pressure, ...). But for the time being it is the format of choice to exchange *aircraft* for formation flights, recorded on different computers (e.g. in a multiplayer session).

### IGC - International Gliding Commission

### GPX - GPS Exchange Format

#### Options
The *Waypoints* option specifies which GPX element (\<wpt>, \<rte>  or \<trk>) to interpret as *waypoint*. Those positions are then stored in the *flight* as *waypoints*.

- **Waypoint \<wpt>** - the \<wpt> elements are interpreted as *waypoints*
- **Route \<rte>** - the \<rte> elements are interpreted as *waypoints* (default)
- **Track \<trk>** - the \<trk> elements are interpreted as *waypoints* (less useful, but possible)

The *Position* option specifies which GPX element (\<wpt>, \<rte>  or \<trk>) to interpret as *position*. The imported latitude, longitude and altitude values are then stored for the *aircraft* as *positions*.

- **Waypoint \<wpt>** - the \<wpt> elements are interpreted as *positions*
- **Route \<rte>** - the \<rte> elements are interpreted as *positions*
- **Track \<trk>** - the \<trk> elements are interpreted as *positions* (default)

So while the default settings interpret the GPX elements as specified ("as intended by the specification") it is possible to import GPX data containing only a (planned) route (\<rte>). So while those points are likely to be few, with a great distance between each, Sky Dolly will do its best to reconstruct the (planned) flight.

> The popular Little Navmap application is able to export flight plans as GPX. The option to specify the *Positions* may be useful in such a use case.

#### KML - Keyhole Markup Language
The *Format* option specifies the KML format (flavour) respectively the source which created the KML file in the first place.

- **flightaware** - the KML file as exported from flightaware.com
- **flightradar24** - the KML file as exported from flightradar24.com
- **Generic track** - the KML file with a &gt;gx:track&lt; element

## Export
In order to export a flight:

- In the *logbook* (key **F1**), load the desired *flight*
- Choose the desired export format via the *File \| Export* menu entry

An export dialog will appear, with common and plugin specific export options.

### Common Export Options

#### Formation Export
The *formation export* option controls how formation flights are to be exported:

- **User aircraft only** - only the *aircraft* designated in the *formation* as *user aircraft* is exported
- **All aircraft (single file)** - all *aircraft* of the *formation* are exported, if possible into a single file
- **All aircraft (separate files)** - all *aircraft* of the *formation* are exported, each into its own file

Some file formats such as KML and GPX support multiple tracks, so they are suitable for exporting a whole *formation* into a single file. Otherwise each *aircraft* is exported into its distinct file, with an enumerated file name starting from 1 to N (the number of formation aircraft).

#### Sample Rate
Resampling is the process of calculating data points at a different frequency (in Hertz) than the original sample rate (also in Hertz). This involves interpolation of the original sampled values, to get another "smooth interpolation curve" through the new sample points. Various interpolation methods exist, such as linear and cubic interpolation.

Sky Dolly applies cubic spline interpolation (Hermite splines) for flight position, velocity and acceleration and mostly linear interpolation for instrument lever positions and flaps positions, for instance.

There are different scenarios where resampling is useful. The two major cases are upsampling (the data is evaluated at a *higher* frequency than the original sample rate) and downsampling (the data is evaluated at a *lower* frequency than the original sample rate):

- Upsampling is done if the recorded data has been sampled only every second (1 Hz), but we need the data for every simulated frame (essentially in the same range as the displayed frames per second, but possibly even higher). So the data is resampled with, say, 60 Hz.
- Downsampling can be done in order to reduce the amount of data. For instance if an aircraft was recorded in the flight simulator for every *simulated frame* (typically in the same range as the displayed frames per second, possibly even higher), but for data analysis reasons it may be sufficient to have the aircraft position sampled only every second (1 Hz). In essence that means a data reduction by a factor of 60 in this example.

The sample rate option specifies the resample rate in Hertz:

- **1/10 Hz** (smallest resulting file size), ..., **10 Hz** (largest resulting file size)
- **Original data (no resampling)** - the original recorded data is exported, without any resampling

> A sample rate of 1 Hz is typically a good choice when it comes to visualising or even analysing flight paths. Choose original data in cases where you want to transfer data from one Sky Dolly instance to another one.

### CSV - Comma-Separated Values

#### Options
The **Format** option let's you choose between various column layouts:

- **Sky Dolly** - all simulation variables supported by Sky Dolly are exported
- **FlightRadar24** - the CSV has the same columns and values as also exported by flightradar24.com

### GPX - GPS Exchange Format

#### Options
The *Timestamps* option let's you choose the base time of the exported timestamps:

- **Simulation time** - the time as set in the flight simulator is taken as start time
- **Recording (real-world) time** - the actual real-world time is taken as start time (the point in time when the recording was started)

> The option *Recording (real-world) time* may become useful if screenshots are taken during flight recording. With the help of the exported GPX with real-world timestamps the photos may then be "geotagged" with the help of other third-party software.

### IGC - International Gliding Commission

#### Options
- The *Pilot name* specifies the name of the first pilot. The name is exported in the header (H record: "pilot in charge").
- The "Co-pilot name* specifies the name of the second pilot. The name is exported in the header (H record: "crew 2")

### JSON - JavaScript Object Notation

#### Options

### KML - Keyhole Markup Language

#### Options

[Index](index.md) \| [Import and Export](import-export.md)
