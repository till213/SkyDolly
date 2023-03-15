# Changelog

## 0.17.0

### Improvements
- The default logbook name is now *Sky Dolly Logbook*
  * In order to help to avoid confusion between the Sky Dolly application and logbook directory
  * Existing logbooks remain at their current location
- The flight number is now also shown in the Logbook table [[Issue #103](https://github.com/till213/SkyDolly/issues/103)]
  * Flights can be also filtered with the flight number
  * The flight number is now stored per flight (not per aircraft anymore)

### Other
- Remove import & export CSV Sky Dolly format
  * The default import CSV format is now: *Flightradar24*
  * The default export CSV format is now: *Position and attitude*
  * Use the Sky Dolly logbook import & export instead
  
### Bug Fixes
- KML export: the _first_ position point is now properly exported as well

## 0.16.2

### Bug Fixes
- Fix the Little Navmap userpoint (location) import
  * Relax CSV validation (import CSV without header)
  * Properly check for empty (value: "") elevation values

## 0.16.1

### Bug Fixes
- Properly store the newly selected logbook file path in the application settings in case an attempt is made to open a logbook that has been written by a newer Sky Dolly version and the user chooses an alternate logbook
  * Regression introduced with v0.16.0

## 0.16.0

**"Gregarious Gee Bee"** introduces new Sky Dolly logbook export & import plugins, incuding time synchronisation for formation flights.

### New Features
- Sky Dolly logbook (SDLOG) export plugin [[Issue #7]5(https://github.com/till213/SkyDolly/issues/75)]
  * The current plugin implementation always exports the entire flight (including all aircraft) into a single logbook file, without resampling
- Sky Dolly logbook (SDLOG) import plugin
  * All flights including all aircraft are imported, including flight information (title, comment, creation date) and flight conditions (temperature, wind conditions, runway conditions, ...)

**Note:** the new Sky Dolly logbook import & export plugins render the existing Sky Dolly CSV import & export obsolete: the Sky Dolly CSV import & export will be removed in an upcoming release. However other CSV import & export formats such as the [flightradar24.com](https://www.flightradar24.com/), FlightRecorder and *position and attitude* formats will remain.
  
### Improvements
- All export plugins: now only the effectively *supported* resampling and formation export options (as reported by the given plugin) are selectable
- All import plugins:
  * The "Add to current flight" checkbox has been replaced by an aircraft import mode combobox:
    - **Add to current flight**: all imported aircraft are added to the currently loaded flight
    - **Add to new flight**: all imported aircraft are added to a flight that is newly created
    - **Separate flights**: a new flight is created for each imported file
  * The aircraft type selection combobox is now disabled for those formats (such as the new Sky Dolly logbook) that already specify the aircraft type
  * Depending on whether the given plugin also reports the flight recording time the timestamp offset of newly imported aircraft can now be automatically adjusted:
    - **None**: no timestamp offset is being applied
    - **Date and time**: the timestamp offset is calculated based on the recording date and time difference between the *current* and each newly imported flight
    - **Time only**: the timestamp offset is calculated based on the recording time difference between the *current* and each newly imported flight, but ignoring the recording date. This can be useful when importing historic real-world flights from different dates in the past into the same "formation flight", but with the goal to replay them "on the same day" (as defined by the first imported flight)
  * Old plugin settings are migrated to the new import mode settings
- The Simulation Variables dialog (key **V**) now also shows the aircraft position as decimal latitude & longitude values [[Issue #89](https://github.com/till213/SkyDolly/issues/89)]
- The flight recording time in the Flight dialog (key **F**) is now empty for new flights (without any recording)
  * As before it is updated when the *first* aircraft is being recorded
  * The recording time text field tooltip now also shows the recording date and time converted to UTC ("zulu time"), including seconds
- The flight title and description are now also editable during recording. Because why not :)
- It is now possible to clear a flight via the menu Replay | Clear. Clearing a flight will simply unload it from memory, that is any persisted data in the logbook won't be modified
- A new recent logbooks menu allows to quickly re-open logbooks that have been opened in the past
- Drag and drop support: you can drag a Sky Dolly logbook from the Windows File Explorer onto the Sky Dolly application in order to quickly open that logbook
- The current logbook file name is now shown in the application window title
- The filter values in the Logbook module are now persisted in the application settings
- The filter and default values in the Formation module are now persisted in the application settings
- The filter values in the Location module are now automatically reset when a user location is being added, in order to make the newly added location visible
- Added five new New Zealand locations

### Bug Fixes
- The timestamp for waypoints is now properly restored when reading waypoint data from the logbook
- The first aircraft position data is properly cached (not reset when timestamp is set to 0)
- Fix the [flightradar24.com](https://www.flightradar24.com/) CSV import (wrong CSV data validation)
- Fix interpolation of first sample point in case the timestamp of the first sample point is > 0 
  * which may occasionally happen if position data arrives a few milliseconds ("one frame") after e.g. the first engine data sample arrives (the arrival of the first sample data starts the timer)
- Fix internal SimConnect exceptions due to duplicate "frame" systemt event subscriptions

### Under the Hood
- Flight import plugin API simplified, by introducing a new FlightData structure that allows to move flight data between Flight objects
- Modules now have module-specifc settings (like import & export plugins)
- Basic unit tests for selected import plugins

## 0.15.1

### Bug Fixes
- Prevents the creation of a logbook backup each time the application is launched:
  * Make sure each "bump version" migration step _really_ has a distinct UUID
  * Don't compare patch version numbers, as the database version does not increment them
- Fix a crash that may occasionally occur when quitting Sky Dolly, in case when MSFS has been closed before and an "active connection" had been established, e.g. due to a replay or recording

## 0.15.0

**"Flamboyant Fokker"** focuses on the migration to the Qt 6.4 framework. If you are building Sky Dolly from source then also refer to the updated [BUILD.md](BUILD.md) instructions.

### Improvements
- The new Qt 6 framework also provides initial support for "dark mode"
  * As this is still considered "work in progress" in Qt 6.4 the "dark mode" is not enabled by default
  * However you can already enable it by providing the following command line arguments:
    - For the "Windows" style:
      * `SkyDolly.exe -platform windows:darkmode=2`
    - For the "Fusion" style that currently looks better in "dark mode":
      * `SkyDolly.exe -platform windows:darkmode=2 -style=fusion`
  * Once the option "darkmode=2" has been set Sky Dolly will then follow the Windows "dark mode" settings
  * Unfortunately the "Windows Vista" style (default style on Windows) won't support "dark mode" (due to underlying Win32 API limitations - also refer to [QTBUG-72028](https://bugreports.qt.io/browse/QTBUG-72028)
- The internal event state is now reset whenever replay is resumed
  * This improves the situation where the flaps handle lever is initially in the wrong position due to multiple consecutive "timeline seek" operations
  * The flaps handle lever will now go into the correct position during replay, upon the next "flaps event"
  * Note that this is due to the "asynchronous nature" ("real-time update") of the _flaps handle index_ simulation variable update that some aircraft have in place (specifically the PMDG 737)

### Under The Hood
- Upgrade Qt to version 6.4.2 (from version 5.15.2)
- Upgrade GeographicLib to version 2.1.2 (from version 1.52)
- Upgrade ordered-map to version 1.1.0 (from version 1.0.0)
- Use the C++20 standard (from C++17)
- Use named database connections: foundation import/export plugins that use the database (Sky Dolly logbook, *.sdlog) format

### Bug Fixes
- "Orphaned create AI object" requests are now properly handled (the just created AI object is destroyed again) when the original request has already been removed in the meantime (e.g. due to a failed aircraft import)
- When importing a file (e.g. CSV format) without waypoint information the created waypoints have now guaranteed a unique timestamp
  * Specifically in the case when only one position sample ("parked aircraft") exist
- Properly restore the default "show replay speed" option (applied when entering minimal UI mode) from the application settings

## 0.14.0

**"Ethereal Embraer"** focuses on improved 3rd-party aircraft support, especially "for the big birds".

### Improvements
- Improved replay specifically with 3rd party aircraft [[Issue #73](https://github.com/till213/SkyDolly/issues/73)], for:
  * Primary and secondary flight controls (ailerons, flaps, spoilers, ...)
  * Engine (thrust lever)
  * Aircraft handles (gear, wing folding, tailhook, ...)
- Improved replay for AI aircraft ("formation flight"):
  * Animated ailerons, elevator and rudder
  * Less SimConnect data sent
- New location categories, including improved CSV import & export with Little Navmap v2.8.8
  * Addon, Cabin, Carrier, History, Obstacle, Oil Platform, Settlement
- When pausing recording the simulation is now also paused
- No need to "repeat flaps" in settings anymore (improved secondary flight controls replay)
- Added new flight information simulation variables (dialog Flight -> Conditions):
  * Surface condition (normal, wet, icy, snow)  
  * On any runway
  * On parking spot
- Increasing the replay speed now also increases the simulation rate (up to a maximum rate of 16x)
  * The simulation rate is a power of 2: 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0 - the nearest simulation rate
    is chosen, given the current replay speed
  * Note: certain aircraft may reduce or even fix the simulation rate to 1.0 (for instance the Fenix A320)
- When the 'Add to flight' option is enabled and import fails then no empty (no recorded data) aircraft is left in the formation
 
### Under The Hood
- Small performance improvements

### Bug Fixes
- Fix "ailerons reversal" (Asobo A320neo, F/A-18 and other aircraft with "PID controllers")
  * Actual root cause (in MSFS/SimConnect): MSFS seems to expect "*feet* per second" instead of "*radians* per second" for "*rotation* velocity body" (*radians* per second would actually be the correct physical unit for rotation velocity)
- Ensure that all "position" and "percent" values as reported by MSFS are clamped to the proper range ([-1.0, 1.0] and [0, 100] respectively)
- Fix a crash in the Formation module that would occur when trying to delete the last aircraft from the aircraft table

## 0.13.1

### Bug Fixes
- The mixture lever position is now properly initialised from "percent" (instead of "position") values:
  * during "flight augmentation" ("flight augmentation" is applied when importing flights from e.g. [flightradar24.com](https://www.flightradar24.com/) or [flightaware.com](https://flightaware.com))
  * during IGC import (when detecting "engine noise" in the IGC data)
- In the Simulation Variables dialog (and elsewhere) ensure that at least one recorded data sample exist before accessing the "last" entry
- Prevent access to invalidated data upon application termination when either the Flight, Simulation Variables or Statistics dialog was open before quitting the application (rare race condition)
- Properly spell "airport" in the location CSV export in Little Navmap format (fix typo)

## 0.13.0

**"Dapper Daher"** introduced location import and export.

### New Features
- New location export plugin
  * Sky Dolly CSV export
  * [Little Navmap](https://albar965.github.io/littlenavmap.html) user point CSV export
- New location import plugin
  * Sky Dolly CSV import
  * [Little Navmap](https://albar965.github.io/littlenavmap.html) user point CSV import
  
### Improvements
- Flight CSV export
  * The Sky Dolly CSV export now uses comma (,) instead of tab stops as value delimiters
  * Size of file reduced, rounding numbers to two decimal places (except latitude and longitude values that are rounded to six decimal places)
- Location module
  * A new update button which updates the selected location with the current location in the flight simulator (sytem locations cannot be updated)
  * Automatically scrolls to the newly inserted item (with any column sorting enabled)
  * Additional system locations (101 in total)
  * Locations can now be filtered with keywords, by country and category
  * The location count is shown
- The system locations migration file, located at *[Sky Dolly installation directory]/Resources/migr/Locations.csv*, is now optional and can be deleted
- A new "Only this time" backup option has been added to the backup dialog: selecting this option will create a backup, but will then set the backup period to "Never" afterwards
- Added default aircraft types from simulation update 11 ("40th anniversary update"), for aircraft selector combobox (flight import)
- Position data is now interpolated within an "infinite interpolation window"
  * Imported flight plans (e.g. GPX flight plans from Little Navmap) with "sparse waypoints" are now properly replayed
- Flight Recorder CSV import now also imports "propeller lever position"
- The keyword search text line edit widgets now have a "clear text" button

### Bug Fixes
- The various tabs in the Flight information dialog (Description, Aircraft, Conditions and Flight Plan) are now properly updated when importing a flight or loading a flight from the logbook
- Imported "Historical flights" - that is to say, any real-world flight done before August 2020 - are now properly shown in the Logbook
  * While the date August 18 2020, the birthday of MSFS, was certainly a reasonable choice for recorded flights...
  * ... the date December 17 1903, the day of the first flight in human history, is certainly a safer bet when it comes to tracked (and imported) flights ;)
- Correct country name of Qatar
- Prevent teleportation by double-click on location ID during replay
- Fixed the TAB order in the Logbook and Location module
- AI aircraft data structures are now properly removed upon disconnect from MSFS: AI aircraft are hence properly re-created when re-connecting to the newly launched MSFS

### Under The Hood
- Rule of Zero, copy-and-swap [[Back to Basics: RAII and the Rule of Zero - Arthur O'Dwyer - CppCon 2019](https://www.youtube.com/watch?v=7Qgd9B1KuMQ)]
- Thread-safe access to singletons [[Back to Basics: Concurrency - Arthur O'Dwyer - CppCon 2020](https://www.youtube.com/watch?v=F6Ipn7gCOsY)]
- "Const pimpl" pattern (where applicable) [[CppCon 2016: Herb Sutter “Leak-Freedom in C++... By Default.”](https://www.youtube.com/watch?v=JfmTagWcqoE)]
- Value-based aircraft list (CPU cache optimisation)
- Small performance optimisations such as return value optimisation and memory pre-allocation
- Replace regular expression etc. based CSV parsers by new CSV parser (very much inspired by [Little Navmap CSV parser implementation](https://github.com/albar965/atools))

## 0.12.0

**"Celestial Cessna"** introduces the "Location" module.

### New Features
- Location module
  * Capture and teleport to locations in the flight simulator
  * Manually add and edit locations
  * Comes with an intial set of "system" locations (currently non-deleteable by design)
  * Stores also pitch, bank, heading and initial speed
  * Country list according to [List of Country Codes [Wikipedia]](https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes)
  * Copy coordinates e.g. from Google maps and paste them (CTRL + V) into the Location module
    - Various supported formats, e.g.
      * 46.94809 7.44744
      * 46.94809, 7.44744
      * 46°56'53.12" N 7°26'50.78" E
      * 46°56'53.12" N, 7°26'50.78" E
      * 7°26'50.78" E 46°56'53.12" N
      * 7°26'50.78" E, 46°56'53.12" N
      * And other variants of the DMS notation

### Improvements
- Renamed 'velocity' (= a vector) to 'speed' (= a value) where appropriate
- When starting a new recording the user aircraft is now updated in the logbook as soon as it becomes available
- The aircraft table in the Formation module is now updated by row (instead of the entire table) upon time offset changes

### Bug Fixes
- The window is properly resized when in minimal mode and the default non-essential button visibility is changed in the settings
- Selection in the Logbook and Formation modules is now properly taking table reordering into account
- The Formation module is properly updated when a new logbook is opened or created
- Formation module: manually adjusted table column is kept when recording a new aircraft

### Under The Hood
- The modules (Logbook, Formation, Location) are now implemented as plugins
- The modules are [topologically sorted](https://en.wikipedia.org/wiki/Topological_sorting), based on their defined dependencies
- "Persisted enumerations": dropdown comboboxes are automatically populated based on the persisted enumeration values
- Sky Dolly now also compiles and links with the Microsoft Visual Studio 2022 C++ compiler (MSVC)
- The code is now also [analysed](https://github.com/till213/SkyDolly/actions) for programming / security flaws with the [Microsoft C++ Code Analysis Action](https://github.com/marketplace/actions/microsoft-c-code-analysis-action)

## 0.11.1

### Bug Fixes
- The maximum time of the replay timer is properly set after recording
- Apply relative position when _Set relative position_ is enabled and _Fly with formation_ is selected

## 0.11.0

This release mostly focuses on "under the hood" improvements, but also improves on existing functionality like the AI aircraft management.

### Improvements

#### Import & Export
- No sequence numbers added anymore to export filename in case the flight has only one aircraft and _All aircraft_ is selected
- IGC & GXP import
  * An option to convert from the WGS84 reference ellipsoid to the earth gravity model (EGM2008) geoid altitude has been added (enabled by default)
  * Note: both the IGC and GPX specifications state that the GNSS altitude (elevation) refer to the WGS84 reference ellipsoid, but not every IGC/GPX file necessarily conforms to the specification and may contain earth gravity model (EGM) geoid altitudes (_above sea level_) instead
- IGC & GPX export
  * The altitude is converted from the geoid (EGM2008) to height above the WGS84 reference ellipsoid
- CSV export
  - A new _position and attitude_ format has been added, exporting the position, pitch, bank, heading and speed, including UTC timestamps

#### Logbook Module
- The order of the logbook table columns can now be changed
- The flight being recorded is now also shown in the logbook: the _red airplane_ indicates this (in analogy to the Formation module)

#### Formation Module
- Improved AI aircraft management
  * Minimised create / delete AI aircraft requests when switching the user aircraft or removing an aircraft
- A new _set relative position_ option allows to keep the user aircraft at its current position, instead of placing it relative to the current formation user (reference) aircraft upon recording or replay
  * When the _set relative position_ option is unchecked then the current user aircraft position remains unchanged when
    - Recording starts
    - The reference aircraft in the formation is changed
    - The relative bearing or distance is changed
  * Otherwise the current user aircraft is positioned in relation to the reference aircraft
  * So if you ever wanted to record two aircraft playing "chicken" by starting from opposite ends of some runway... this is your opportunity ;)
- The order of the formation aircraft table columns can now be changed
- The _Delete_ key now deletes the selected aircraft
- The reference aircraft in the formation table changes colour to _green_ when setting the replay mode to _fly with formation_, indicating that the user controls an "extra" user aircraft (which is initially placed relative to the green reference aircraft)
- Minor layout adjustments

#### Minimal UI Mode
- Button texts, non-essential buttons and the replay speed are now automatically hidden when switching into minimal UI mode
- This behaviour (the default visibility while in the minimal UI mode) can be defined with new options in the Settings | User Interface section
- The replay speed visibility can now also be toggled while in minimal UI mode

#### User Interface
- Latitude and longitude values are now formatted with leading zeroes
- The recording timer is now always incremented, even (especially) when no sample data is currently being recorded (no value changes, e.g. typically for a "cold and dark" aircraft)
- Added and improved tooltips
- All question and information dialogs now have an explanatory title
- Tables now have alternating colours per row
- A question and confirmation dialog is now shown before and after logbook optimisation, also showing the before and after file sizes (after the optimisation)
- Dialogs (Flight, Simulation Variables, Statistics) are now (only) created on demand and immediatelly released after use, resulting in a smaller memory footprint
- The About dialog
  * Now contains references to used third party libraries and tools
  * Allows to copy the (extended) version information into the clipboard by clicking onto it
- Small layout improvements

### Bug Fixes
- In the logbook module the date selection column width is now properly calculated (no month name is truncated)
- The position slider is now properly updated whenever an aircraft is removed or its time offset has been changed
- When pausing a recording (yes, that's actually possible ;)) the "recording timer" is only restarted in case no event-based recording is done (but a "timer-based recording" instead)
- The Window | Minimal menu entry is now properly synchronised (unchecked) when the module is changed while in "minimal UI" mode (e.g. by pressing F1 or F2, or via the Module menu)
- File paths are converted to native separators when shown in dialog boxes to the user
- The Settings dialog is now created on demand. This also fixes an issue with dialog z-ordering when the main application window is switched to "Stay on Top"
- Depending on the selected module the persistence of a flight is different (single aircraft vs formation flight). In order not to mess up the persistence logic switching modules is now disabled while recording 
- In order to prevent data loss when recording a flight the following functionality is now properly disabled during recording:
  * No existing flight can be loaded (including by double-clicking onto a logbook row)
  * No user aircraft can be switched in the formation (including by double-cicking onto an aircraft in the formation table)
  * Creating a new logbook or opening an existing one is disabled
  * Quitting the application (with CTRL + Q or File | Quit) will properly stop and persist an ongoing recording

### Under The Hood
- Introduction of the GeographicLib (https://geographiclib.sourceforge.io/)
  * Calculating undulation values (conversion between geoid and WGS84 reference ellipsoid altitudes)
  * Geodesic calculations (distance, azimuth, relative position)
- Refactored the build system ("modern CMake")
  * Resources like the undulation file can be auomatically (optionally) downloaded at build configuration time (refer to the [BUILD.d](BUILD.md) instructions for details
  * Third-party libraries (git submodules) like the GeographicLib are automatically checked out
  * The environment variable MSFS_SDK is now queried in order to detect the MSFS SDK install path
- Model classes refactoring
  * Template based, in order to reduce repetitive code
  * Qt Signas and slots re-wiring
- Plugin management refactoring
  * Plugin managers are now in a common PLuginManager library
  * Plugins are now built after the main application
    - Especially on macOS this allows to place the plugins inside the previously built application bundle
- And finally... on macOS the application icon set is now automatically generated at build time

## 0.10.0

**"Beasty Boeing"** adds two new export plugins (JSON and GPX) and improves various existing import and export plugins.

### New Features

- JSON export plugin
  * Export in GeoJSON format
  * Including style properties, interpreted e.g by https://geojson.io
- GPX export plugin
  * Timestamps are based on either simulation or recording time; the later being useful for GPS-tagging screenshots taken during flight recording

### Improvements

- All import plugins
  * An entire directory can now be imported at once:
    - All imported files are imported as distinct flights or
    - All imported aircraft are added to the same newly created flight ("formation flight")
- All export plugins
  * File existence is checked before overwriting existing files
  * An export option for formation flights:
    - Only export the current user aircraft
    - All aircraft are exported into a single file (if supported by the file format, e.g. KML, GPX and GeoJSON)
    - All aircraft are exported into separate files
  * Resampling ("original sample rate", "1 Hz", "2 Hz", "5 Hz", ...) is now supported by all export plugins
- CSV export plugin
  * The plugin now also exports [flightradar24.com](https://www.flightradar24.com/) CSV format
- IGC export plugin
  * The true heading (HDT) and indicated airspeed (IAS, in km/h) are now exported (in K records)
- KML export plugin
  * The LineStrings are now exported as connected multi-geometry lines (no point limit anymore)
  * Reduced the number of fractional digits to 6, reducing file size while still providing a practical precision in the range of centimeters (also refer to the mandatory [XKCD](https://xkcd.com/2170/))
- Miscellaneous
  * Add shortcuts to dialog buttons
  * The recording time is now shown in the Flight dialog
  * Use title-case for all [push buttons](https://developer.apple.com/design/human-interface-guidelines/macos/buttons/push-buttons/)
  * Add additional aircraft types (F/A 18, PC-6, Pitts Special, NX Cub, Volocity) from sim update 7 for the import aircraft selection
    - Note: third-party aircraft types are automatically made available in the aircraft selection once recorded at least once
  * The new Help menu contains a link to a "work in progress" [online manual](https://till213.github.io/SkyDolly/manual/en/)
  
### Bug Fixes

- The flight creation date & time is now properly set when a recording (of the first aircraft) _starts_ (not when the flight is persisted only)
- The logbook now properly shows _simulation_ start- and end times (not local times)
- The IGC export plugin now properly exports _simulation_ timestamps (not local timestamps)
  
## 0.9.0

**"Anarchic Airbus"** introduces various new import and export plugins: IGC, GPX and CSV (flightradar24).

### New Features

- IGC export plugin [[Issue #15](https://github.com/till213/SkyDolly/issues/15)]
  * Export flight path (position and GNSS altitude)
  * Export waypoints as flight tasks ("C records")
  * Export pilot names
- GPX import plugin [[Issue #16](https://github.com/till213/SkyDolly/issues/16)]
  * The GPX format contains three elements: waypoints, routes and tracks
    - Waypoints are typically "places to visit" along the way
    - Routes describe "how to get there"
    - Tracks contain the actually travelled path (typically recorded with a GPS device)
  * The import dialog allows to select which elements to use for flight waypoints and the actual flown path
  * It also offers a default altitude and speed, as timestamps and elevation values are optional values in the GPX format
- CSV import plugin
  * The following CSV formats are now supported:
    - CSV from [flightradar24.com](https://www.flightradar24.com/)
    - CSV from [Flight Recorder](https://flightsim.to/file/8163/flight-recorder)
- KML import plugin
  * The following KML formats are now supported:
    - KML from [flightradar24.com](https://www.flightradar24.com/)
    - Generic KML documents containing at least one &lt;gx:Track&gt; element
  
### Improvements

- Import
  * The aircraft selection combobox is now editable, with autocompletion support
  * The selected aircraft is now stored in the settings
- IGC import plugin
  * The initial heading is now calculated on the _first significant movement_, compensating GPS inaccuracies (when the glider is standing still on the ground)
  * Date and time of turn points (waypoints) defined by the task list (_C records_) are now updated with the actual flight time, based on the closest flown position
  * Environmental noise level (ENL) determines the engine state (on/off, propeller lever)
- Logbook
  * The _Delete_ key now deletes the selected flight
  * Columns will only be resized to content upon first data load; afterwards manual column resizing won't be changed
- Replay
  * A replay loop button has been added: when enabled the replay restarts at the beginning whenever the end is reached
  * In the settings the seek interval is now settable with a spinbox instead of a text field
- Recording
  * The simulation variable _indicated airspeed_ is now also recorded (however only for analytical purposes - e.g. exported as _pressure altitude_ in the new IGC export plugin)
  
### Bug Fixes

- The spoiler position value is now properly shown as percent (instead of _raw number_) in the Simulation Variables dialog
  * For clarity the field is now also labeled _spoiler position_ (instead of just _spoiler_)
- Switched internal leading/trailing flaps position unit from _percent_ to _position_
  * The greater precision (32767 vs 255 possible values) gets rid of the _flaps sound stutter_ (e.g. audible in the cockpit of the A320neo)
  
### Miscellaneous

- Sky Dolly wants and promotes peace

## 0.8.6

### New Features

- IGC import plugin
  * The International Gliding Commission (IGC) approved flight recorder format for gliders
  * Basic header data (_H records_) import (pilot names, glider type and ID)
  * Basic position (_B records (fix)_) import
  * Basic task (_C records_) import (waypoints)

### Bug Fixes

- Typo: plural of aircraft is aircraft (credits: Stefano Caporelli, Youtube comment [Sky Dolly - Formation Flying](https://www.youtube.com/watch?v=Op_zTfM3-HQ&lc=UgyZXpfBxZ5iLpqnd5F4AaABAg))
- KML import plugin
  * _Flight augmentation_ (adding flight procedures, aircraft attitude etc.) does properly terminate when imported flight duration is less than 3 minutes
  * Validate the number of imported positions
  * Show a simple error message in case of import failure

## 0.8.5

### New Features

- KML import plugin which imports KML flight data from [flightaware.com](https://flightaware.com)

### Bug Fixes

- Dates and times are now properly displayed using the system locale (e.g. "8/5/2021" or "5.8.2021" for dates like "August 5 2021",
  "5:35 PM" or "17:35" for times)
  * Date and time table items are now also properly sorted, regardless of their _locale representation_
- Logbook: the date column of the _date chooser widget_ is now properly resized according to its content (month names)

## 0.8.4

### Bug Fixes

- The initial speed when recording a new formation aircraft is now properly set (indicated airspeed, as opposed to true airspeed)
  * Note that the conversion from true to indicated airspeed is currently done with a _rule of thumb_ only
  * https://www.pilotmall.com/blogs/news/how-to-calculate-true-airspeed-and-what-it-is-guide
  * Depending on the altitude and especially pressure there might still be a substantial difference to the actual indicated airspeed
- Export
  * Illegal file path characters are replaced by _ in the initially suggested file path
  * KML export: exported character data (title, description) is properly enclosed in a CDATA tag
- Properly update AI aircraft when deleting the user aircraft during replay

## 0.8.3

### Bug Fixes

- The main window is now made a _parent_ of the KML export dialog, making sure that the export dialog is always centered and on top of the main window
  * Especially when the _Stay on Top_ option is enabled for the main window
- The initial speed in _fly with formation_ replay mode is now properly set
  * Properly converted from feet/s to knots
  * Properly converted from true to indicated airspeed (the SIMCONNECT_DATA_INITPOSITION structure really seems to expected indicated airspeed)

## 0.8.2

### Bug Fixes

- Starting the recording of the first aircraft directly in the Formation module would _teleport_ the aircraft to be recorded right to the North Pole (0/0 longitude/latitude)
  * The _relative initial recording position_ calculation now properly checks the returned _null position_ value

## 0.8.1

### Bug Fixes

- Creating new logbooks works again
  * an attempt was made to create a backup before migration of a non-existing database

## 0.8.0

This release introduces a new plugin architecture for import and export plugins.

### New Features

- Plugin architecture
  * Import CSV plugin
    - Aircraft can now be _added_ to existing flights (_multiplayer formation flying_)
    - Select the aircraft type (used when spawning AI aircraft in formation flights)
    - Aircraft types (name, number of engines, wingspan, ...) are constantly updated upon flight recordings
      * The default aircraft of MSFS are already in the logbook and hence immediatelly selectable when importing a CSV file
    - Also refer to new _time offset_ feature in the Formation module which is useful to _synchronise multiplayer formation flights_, see below
  * Export CSV plugin
  * Export KML plugin ("Google Earth")
    - Basic line style settings
    - Plugin settings are persisted in Sky Dolly application settings
  * SkyConnect plugin architecture
    - SimConnect plugin (currently FS2020 only)
    - Auto-detection of installed flight simulator (automatic plugin selection)
    - Plugin changeable at runtime via Settings dialog
  
#### Modules

- Logbook
  * Filtering of flights 
    - By date via creation date tree widget
    - Search field which matches the flight title, user aircraft and destinations
    - Engine type (jet, piston, turboprop, ...)
    - Formation flights (flights having more than one aircraft)
  * The selected _user aircraft_ is now shown in the flight log table (previously: simply the first recorded aircraft of the formation was shown)
- Formation
  * A time offset can be added to the selected aircraft, effectively _shifting_ it back or forth on the timeline
    - This is useful for _multiplayer formation flights_ in order to synchronise the imported aircraft (also see CSV import plugin above)
  * A new _fly with formation_ replay mode has been added:
    - All previously recorded aircraft are replayed
    - You fly with your currently loaded user aircraft with the formation (_in addition_ to the recorded aircraft)
  * _Take control of recorded user aircraft_ still exists
    - You can now switch control between user aircraft (also during replay), by double-clicking on the aircraft in the _Formation_ table (or push the _Set User Aircraft_ button)
  * Pause will now also _freeze_ the user aircraft
    - For both _fly with formation_ and _Take control_ replay modes
    - Additionally in _fly with formation_ replay mode user aircraft can be re-positioned relative to the _recorded_ user aircraft in the formation while paused, by clicking on one of the _relative positions_
  * Aircraft can now also be deleted from a formation flight during (paused) replay

### Improvements

- Better support for aircraft engine start & shutdown
- Additional simulation variables
  - Combustion
  - Smoke enabled/disabled (at the time of this writing not yet supported by FS2020)
- Database performance improvements:
  * _Forward queries_
  * _Normalisation_ of _aircraft type_ data (new table - less storage space required)
- A new _View_ menu has been added, providing options to show/hide various UI elements such as the module selector
- Logbook backup can now be scheduled, the logbook settings are now in the _File_ menu
  * Once a month
  * Once a week
  * Daily
  * Never or whenever exiting Sky Dolly
- Older logbooks are not backed up before migration
  * This can be disabled in the new Logbook Settings (File/Logbook Settings...)
- User interface and usability refinments
  * _Active_ icons
  * Small layout optimisations
  * Creating a new logbook now changes into parent directory of the actual logbook directory, when logbook file name (without extension) corresponds with the parent directory name
    - E.g. when creating a new logbook, when the existing one is _c:\Users\The User\Documents\Sky Dolly\Sky Dolly.sdlog_, the file dialog will now start with _c:\Users\The User\Documents\_
    - (Previously: _c:\Users\The User\Documents\Sky Dolly\_)
  * The replay speed group can now be hidden has well, just like the module selector
    - Both view options are now in the new View menu
    - Hiding the replay speed group and then switching into _minimal UI_ mode (key M) allows for a truly minimalistic UI now
    - Note that the module selector and replay speed group view visibiliites cannot be toggled while in _minimal UI_: switch to normal UI (key M) first
- Sky Dolly logbooks (*.sdlog) can now be associated with the Sky Dolly application: they will be opened upon double-click on the logbook (file)
  * Associating *.sdlog files with SkyDolly.exe needs to be manually done for now (right-click on file, _Open with..._, choose SkyDolly.exe)
- Pressing the same shortcut for the Show Flight|Simulation Variables|Statistics (keys F, V and S respectively) now closes the corresonding dialog again

### Bug Fixes

- Proper _HiDPI support_ on Windows

## 0.7.1

### Bug Fixes

- Only the database version is now read from the metadata table when comparing versions
  * Related metadata tables and columns may change in future versions, resulting in an SQL error when trying to read the entire metadata table

## 0.7.0

This release introduces the Formation module.

### New Features

- New module: Formation - record an unlimited number of aircraft and replay them altogher, with one single user interface
  * Simply add new aircraft to the formation by clicking the _record_ button while in the Formation module (the plus (+) sign inidcates that aircraft are being added)
  * Previously recorded aircraft are replayed during recording of the new aircraft
  * Change the user aircraft (the one which is being followed by the camera) at any time during replay
  * Take control over the user aircraft any time during replay: manually fly it at any time (and give control back to replay at any time)
  * Delete aircraft from a formation
  * Set the position (relative to the current user aircraft) for the new aircraft to be recorded
- All aircraft are automatically persisted and added to the current flight
- The logbook now shows the number of aircraft per flight
- Start a new flight (starting with a single aircraft) by switching into the Logbook module
- The existing Simulation Variables and Flight dialogs show data about the currently selected user aircraft
- The module selector may be hidden
- Reduced data size of requested simulation variables

### Bug Fixes

- Make sure that the _delete flight_ confirmation dialog is on top of the main window (specifically when the _stay on top_ option is enabled)

## 0.6.3

### Bug Fixes

- Only the database version is now read from the metadata table when comparing versions
  * Related metadata tables and columns may change in future versions, resulting in an SQL error when trying to read the entire metadata table

## 0.6.2

### New Features

- The logbook (database) version is compared with the current Sky Dolly application version
  * Newer logbooks are not opened with older Sky Dolly applications
  * The user is offered the options to either create a new or open another (existing) logbook, or to cancel the operation

### Bug Fixes

- The logbook table (in the user interface) is cleared upon logbook (database) disconnection
- Decimal points in logbook file names are not _swallowed_ anymore (e.g. _My Logbook v0.6.sdlog_ is a perfectly fine filename)
- UI elements such as Record and CSV Import/Export are now properly enabled or disabled based on the logbook (database) connection status

## 0.6.1

### Bug Fixes

- Fix "logboog" typo in message
- Improve minimal UI switching
- Don't reset the elapsed time to 0 when unpausing a recording
- Keep displayed time when pausing a replay (don't set to total replay time)
- Automatically connect with simulator when seeking position
- Also pause the initially loaded flight
- When _repeat values_ is enabled then also repeat values with _zero value_ (e.g. _canopy closed_)
- Automatically reconnect in case the SimConnect server (flight simulator) has crashed
- Also store the flight into the logbook if a paused recording is stopped
- Don't request SIMCONNECT_PERIOD_NEVER twice (e.g. when stopping a paused recording): prevent SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID server exception
- Ensure that waypoints have distinct timestamp (prevent database unique constraint violation)

## 0.6.0

This release introdcues the logbook, an SQLite based database.

### New Features

- Add SQLite database support (_logbook_) for persistence
  * Each flight is automatically persisted
  * Simple load / delete functionality
  * DB backup and optimisation functionality
- Simple logbook table which lists the recorded flights
- Each flight has an editable title and description
- The previous _Scenario_ is now called a _Flight_ (_Flight_ dialog, shortcut key: F)
- Additional simulation variables support
  * Waypoints (ICAO, latitude/longitude/altitude) are shown in the order of approach; each waypoint is only counted once
  * Local and zulu simulation time
- Replay speed now also available in replay menu, including shortcuts: CTRL + [1 - 4] for slow motion, SHIFT + [1 - 5] for timelapse
- The custom replay speed factor can now be entered either as an absolute factor (1.0 = normal speed) or as percent (100% = normal speed)
- Remove _Yoke X|Y Position_ simulation variables (not needed: yokes are still properly animated)
- Option to enable a _minimal UI_ (shortcut key: M)
- The window geometry and state are persisted and restored upon application start

## 0.5.2

### New Features

- New _repeat values_ settings for selected simulation variables have been introduced: canopy and flaps  
  * Certain aircraft counteract the requested simulation variables, e.g. the canopy closes or the flaps retract automatically
  * So with the _repeat_ option enabled and if the given simulation variable has a value greater zero (e.g. _canopy open_) then its value is repeatedly sent to Flight Simulator
  * The optimal settings are to disable the _repeat_ options (in which case simulation variables are only sent when the value actually changes)

### Bug Fixes

- Fixes a crash which would occur when quitting the application and when the Simulation Variables dialog was still open
- Altitude and other simulation variables measured in feet are now properly displayed with their unit ('ft')
- Replaced the non-writeable FOLDING WING HANDLE POSITION simulation variable with FOLDING WING LEFT|RIGHT PERCENT

## 0.5.1

### Bug Fixes

- Properly parse the custom speed factor according to the current system locale (decimal delimiter may be , or .)
- The initial custom replay speed factor is now displayed with decimal places (e.g. 1.00 or 1,00 - to illustrate the expected decimal delimiter according to the system locale)
- Custom speed factor tooltip shows the valid value range
- Properly name simulation variable _Yoke X Position_ (instead of Y)
- Simulation variables are now formatted according to the system locale

## 0.5.0

This release adds support for more simulation variables and reduces the amount of request data by organising the simulation variables into groups.

### New Features
- Simulation variables have been split into distinct requests, saving both on required space and CPU usage
- The replay speed is now a factor instead of percentage value (1 = normal speed, 2 = double speed etc.)
- Additional simulation variables: 
  * Cowl flaps
  * Electrical master battery
  * General engine starter
  * Folding wing handle
- Simulation variables now shown with units (feet, hPa, ...)
- Added a new _Flight_ dialog, showing information about the aircraft and initial flight conditions
- The last CSV export/import directory is stored
- A basic error dialog shows when an import/export error occurs
- The seek interval can now be defined in the settings (either absolute in seconds, or in percent of the record duration)

## 0.4.2

### Bug Fixes

- Do not start the elapsed timer when seeking a play position and not in replay mode (replay does not _skip ahead_ when started)

## 0.4.1

### Bug Fixes

- The time widget is now editable when replay is paused
- After importing a CSV file only pause when in connected state
- Do not start the elapsed timer when skipping to begin of recording
- Properly hide window when stay on top is deselected

## 0.4.0

This release supports additional simulation variables.

### New Features

- Additional simulation variables: 
  * Propeller RPM
  * Mixture lever positions
  * Body Velocity
  * Rotation Body Velocity
  * Tailhook
  * Canopy
  * Lights (navigation, taxi, strobe, ...)
- Reduce required memory for sample points by using integer data types instead of double values for selected simulation variables
- Pause is enabled after CSV data import
- A high precision timer is used to increase the sample accuracy
- A new recording statistics dialog has been added, also showing a recorded samples per second counter
- New icons for record/play control actions
- When recording sample rate is set to 1 Hz (one sample per second) then the SimConnect request period is now also set to 1 Hz
- A new _auto_ recording sample rate has been added 
  * Recording: samples are collected in an event-based manner ("as they arrive")
  * Replay: samples are always sent _per visual frame_
- Reorganise simulation variables in tabbed widget
- Add additional flight information variables, captured once at start of recording
  * Airline (Callsign)
  * Flight number
  * Tail number
  * Wing span
  * And more

### Bug Fixes

- Properly reset play position to begin after importing CSV data
- Validate the CSV data upon import: specifically the first timestamp must be 0 (timestamps are shifted accordingly)
- _Show simulation variables_ action is now deselected when the dialog is closed (via close button)
- Set the initial start position (_on ground_) when replay from (or seek to) start
- Prevent aircraft from _dropping out of the sky_ after replay, by recording body velocities

## 0.3.1

### Bug Fixes

- CSV import: Properly handle platform-specific line endings 

## 0.3.0

This release introduces the first persistence, a simple CSV format.

### New Features

- Additional simulation variables: flap positions, water ruder and brakes
- Improved position seek performance
- Skip to begin, end, forward and backward buttons
- Settings dialog with sample rate settings
- Basic CSV import & export
- Window stay on top option

### Bug Fixes

- Make sure that the first sampled data properly starts with a timestamp of 0, in order to properly interpolate the requested values
- Properly close the connection when the server (flight simulator) quits

## 0.2.1

### Bug Fixes

- Properly store aircraft control simulation variables, such as ailerons, yoke position, throttle, elevators and rudder

## 0.2.0

This release extends the basic recording & replay implementation.

### New Features

- Recording & replay of basic flight controls (rudder, ailerons, flaps, gear, ...)
- Updated user interface
- Separate Simulation Variables dialog
- Simulation variables are now also shown during replay (in the new Simulation Variables dialog)
- Add record/play/pause actions, with keyboard shortcuts (r = record, space = play, p = pause)

### Bug Fixes

- Validate the recorded simulation variables, specifically the position (sending invalid values back freezes or even crashes MSFS)
- Properly store the already elapsed time when changing the replay speed, for a smooth continuation of the replay

## 0.1.0

This release implements basic recording & replay functionality.

### Features

- Basic recording functionality: record, pause
- Basic aircraft data is recorded:
  * Latitude, longitude, altitude
  * Pitch, bank, heading
  * Aircraft info: on ground, model name
- Basic replay functionality: play, pause, select replay position
- Selectable replay speed

### Bug Fixes

- n/a
