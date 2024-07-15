# Sky Dolly - The Black Sheep for Your Flight Recordings

## About

**Sky Dolly** is an external, stand-alone application that let's you record and replay your flights in Microsoft Flight Simulator (MSFS). It can also simultaneously replay previously recorded aircraft during recording, making it in effect possible to iteratively record your own *formation flights*.

Recording is done by storing so-called *simulation variables* (via the official [SimConnect API](https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/SimConnect_API_Reference.htm) such as position, aircraft attitude and other *simulation variables* that represent the aircraft state (gears, flaps, spoilers, ...).

## Modules

A plugin-based architecture makes **Sky Dolly**  memory-efficient (minimal RAM usage) extendable with new functionality. The following plugins - or *modules* - are currently available:

- **Logbook** - stores your flight data in an SQL database ([SQLite](https://www.sqlite.org/index.html)) and provides easy access to your recorded flights, including various filter options

- **Formation** - record formation flights, even with different aircraft types. Already recorded aircraft are replayed while you record your current aircraft

- **Location** - the location module let's you quickly teleport to the selected location. Besides the position you also get control over the engine state (*start*, *shutdown*, *keep*), making it possible to define your own custom take-off points (for instance custom helipads)

**Sky Dolly** comes with over 100 default locations, all finetuned and with verified GPS coordinates. You can easily capture or add your own user locations, e.g. paste any GPS coordinate with **CTRL + V** into the location table.

## Import & Export Plugins

Various file formats can be imported and exported:

## Flight data:

- **SDLOG** - import & export in Sky Dolly logbook format
- **CSV** - import & export, including formats from [flightradar24.com](https://flightradar24.com), [Flight Recorder](https://flightsim.to/file/8163/flight-recorder) (import only) and a simple position & attitude format
- **IGC** - International Gliding Commission - import & export
- **KLM** - import for [flightaware.com](https://flightaware.com), [flightradar24.com](https://flightradar24.com) and generic KML ([Google Earth](https://www.google.com/earth/about/versions/)) export
- **GPX** - import and export of the [GPS exchange format](https://www.topografix.com/gpx.asp)
- **JSON** - [GeoJSON](https://geojson.org/) export

## Locations:

- **SDLOG** - import & export in Sky Dolly logbook format
- **CSV** - import & export, including Sky Dolly and [Little Navmap](https://albar965.github.io/littlenavmap.html) formats

For instance you can literally import all world update POIs and airports and more from e.g. here: [Little Navmap - MSFS POI Database](https://flightsim.to/file/17193/little-navmap-msfs-poi-database) (made available by Rysatko).

## Preview Release

Note that this is an early pre-release version. While it should run stable and all its present features should work as intended it is far from feature complete.

⚠️⚠️⚠️ **IMPORTANT PATCH RELEASE: 0.17.5** ⚠️⚠️⚠️

Sky Dolly **v0.17.1** introduced a regression that would attempt to connect via network (IPv4) with Microsoft Flight Simulator, preventing the recording of new flights in certain cases.

This has been *fixed* with the **0.17.5** patch relase that also corrects existing "off by one" application settings, ensuring that a local ("pipe") connection is made again. Unless explicitly set in the Sky Dolly application settings to one of the two network connection types (either IPv4 or IPv6).

## Earlier Releases

In case you want to still use the previous **Sky Dolly** versions you may download all previous releases from the [github release page](https://github.com/till213/SkyDolly/releases) (the ZIP archives are identical to the latest one available on [flightsim.to](https://flightsim.to/file/9067/sky-dolly)).

## Source Code

The source code may be found at:

[https://github.com/till213/SkyDolly](https://github.com/till213/SkyDolly)

Additional usage information ([README](https://github.com/till213/SkyDolly/blob/main/README.md)) and a complete [CHANGELOG](https://github.com/till213/SkyDolly/blob/main/CHANGELOG.md) are also available in the github.com source repository. 

## Verify Your Download (SHA-256 Checksums)

You may want to compute the SHA-256 checksum of your downloaded ZIP archive and compare it against the [SHA-256 checksums](https://github.com/till213/SkyDolly/blob/main/SHASUM256.md) published on github.com, to verify the file integrity of your downloaded copy.

## Reporting Problems

Problems can be reported here:

[https://github.com/till213/SkyDolly/issues](https://github.com/till213/SkyDolly/issues)

A free github.com account is required though. Smaller issues may also be reported in the comment section on [flightsim.to](https://flightsim.to/file/9067/sky-dolly).

## Installation

- Unzip the ZIP archive to any location of your choice
- Run **SkyDolly.exe**
- *Optional*: you may associate Sky Dolly *logbooks* (files with the extension **\*.sdlog**) with the **Sky Dolly** application:
  * in File Explorer: right-click on the logbook file and choose *Open with...*
  * Select the **SkyDolly.exe** application
  * Make sure to enable the *Always open with...* checkbox
  * The default logbook is located at: *C:\Users\[Your Username]\Documents\Sky Dolly Logbook\*
  * Once the **\*.sdlog** files are associated with **Sky Dolly** you can double-click on those files and a new application instance will start and open the selected logbook

## Basic Usage

- Start a flight in Microsoft Flight Simulator (MSFS)
- In **Sky Dolly**, press the *Record* (key **R**) button
- Stop the recording (either hit key **R** again, or **ESC** for *stop*)
- Press the *Play* button (**spacebar**)
- Seek the play position (**left** and **right arrow keys**, **page up/page down** keys)
- Show flight information (key **F**)
  * Hint: press key **F** again to close the *Flight* information dialog
- Show the recorded *simulation variables* (key **V**)
  * Hint: Press key **V** again to close the *Simulation Variables* dialog
- Toggle the user interface between minimal and normal size (key **M**)
- Switch between the various *modules* with the function keys (**F1**, **F2**, **F3**, ...).

### Load a Flight

- Select the desired flight in the *logbook*
- Press the *Load* button (or double-click the row)
- Note: double-clicking either the *Title* or *Flight Number* column will edit the title respectively flight number

### Add Aircraft to Existing Flight ("Formation Flying")

- Switch to the *Formation* module (press **F2**)
- For each aircraft to be recorded press the *Record* button (press **R**): the plus (+) sign in the *Record* button icon indicates that aircraft will be added to the existing flight

Note again that the existing aircraft will be replayed during recording, so you know exactly where to fly.

- To record a new flight from scratch simply switch back to the *Logbook* module (press **F1**) and press the *Record* button (key **R**)

You may also load an existing flight in the *Logbook* module before switching to the *Formation* module. Once all aircraft have been added start replay, pause and seek in the timeline as usual.

### Formation Module

- Aircraft may be deleted from a flight
- The last recorded aircraft becomes the *user aircraft* (the one being followed with the camera in MSFS)
- Before recording a new aircraft you may set its position relative to the current *user aircraft*
- You may switch the *user aircraft* at any time - also during replay!
- Want to fly for yourself together with the formation?
  * Simply select the *Take control of recorded user aircraft* option in order to take control of the recorded *user aircraft*
  * Or choose the *Fly with formation* to join the formation with the currently loaded *user aircraft* (so you can also join a *single aircraft formation* in an ad-hoc manner)
  
### Database Backup and Optimisation

Database backups can be automatically scheduled (*once per month*, *once per week*, *never*, ...) and are done after exiting the **Sky Dolly** application (where they may also be skipped). Before migration an update is also done. All backup options are now in the *File | Logbook Settings...* dialog.

By default the backup databases will be located in the same folder as the original *logbook*, in a *Backups* subfolder, but can be changed once the *Backup* dialog shows up.

As flights are recorded and deleted the database file may grow considerably. Unused file space will be reclaimed at some point (by [SQLite](https://www.sqlite.org/index.html)), but in case many flights have been deleted the database file can be optimised, by the *Optimise* command located in the *File* menu.

## Recommendations

- Disable *crash detection* and *engine/stress failure* options in MSFS

## Known Issues

- Depending on the aircraft type some controls (spoilers, flaps, gears, ...) are not properly replayed
- The stock helicopters from the "40th anniversary update" are not yet fully supported: while position replay works fine, not all flight controls (handles) are fully recorded.
- Helicpoters cannot be spawned as *AI aircraft*, making them "invisible" in formation flights (this is due to a current [technical limitation in MSFS](https://devsupport.flightsimulator.com/t/su11-spawn-helicopter-as-ai/4905))
- Aircraft in formation flights - so-called AI aircraft - tend to *jitter* when flying nearby
- Flights imported from e.g. flightaware.com (KML) or flightradar24.com (CSV, KML) often have altitudes not aligned with the MSFS scenery, so aircraft either are below or above the runway when taking off or landing
- Bank angle calculation on imported flights is very basic (and will be improved)
- Error handling in Sky Dolly, specifically for import, is currently very minimalistic. In general:
  * Make sure that you have selected the proper "flavour" (data source, e.g. *flightradar24* or *flightaware*) for a given format
  * The generic KML format tries to find a "track" element, but if no such track element is present then the same generic "File could not be imported" error message will be shown
