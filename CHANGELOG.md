# Changelog

## 0.11.0

### Improvements

#### Import & Export
- No sequence numbers added anymore to export filename in case the flight has only one aircraft and "All aircraft" is selected
- IGC & GXP import
  * An option to convert from the WGS84 reference ellipsoid to the earth gravity model (EGM 2008) geoid altitude has been added (enabled by default)
  * Note: both the IGC and GPX specifications state that the GNSS altitude (elevation) refer to the WGS84 reference ellipsoid, but not every IGC/GPX file necessarily conforms to the specification and may contain earth gravity model (EGM) geoid altitudes ("above sea level") instead
- IGC & GPX export
  * The altitude is converted from the geoid (EGM 2008) to height above the WGS84 reference ellipsoid
- CSV export
  - A new "position and attitude" format has been added, exporting the position, pitch, bank, heading and speed, including UTC timestamps

#### Logbook Module
- The order of the logbook table columns can now be changed
- The flight being recorded is now also shown in the logbook: the "red airplane" indicates this (in analogy to the Formation module)

#### Formation Module
- Improved AI aircraft management
  * Minimised create / delete AI aircraft requests when switching the user aircraft or removing an aircraft
- The order of the formation aircraft table columns can now be changed
- The "Delete" key now deletes the selected aircraft
- Minor layout adjustments

#### Minimal UI Mode
- Button texts, non-essential buttons and the replay speed are now automatically hidden when switching into minimal UI mode
- This behaviour (the default visibility while in the minimal UI mode) can be defined with new options in the Settings | User Interface section
- The replay speed visibility can now also be toggled while in minimal UI mode

#### User Interface
- Latitude and longitude values are now formatted with leading zeroes
- The recording timer is now always incremented, even (especially) when no sample data is currently being recorded (no value changes, e.g. typically for a "cold and dark" aircraft)
- Additional tooltips in the settings dialog
- All question and information dialogs now have an explanatory title
- Tables now have alternating colours per row
- A question and confirmation dialog is now shown before and after logbook optimisation, also showing the before and after file sizes (after the optimisation)
- Dialogs (flight, simulation variables, statistics) are now (only) created on demand and immediatelly released after use, resulting in a smaller memory footprint

### Bug Fixes
- In the logbook module the date selection column width is now properly calculated (no month name is truncated)
- The position slider is now properly updated whenever an aircraft is removed or its time offset has been changed
- When pausing a recording (yes, that's actually possible ;)) the "recording timer" is only restarted in case no event-based recording is done (but a "timer-based recording" instead)
- The Window | Minimal menu entry is now properly synchronised (unchecked) when the module is changed while in "minimal UI" mode (e.g. by pressing F1 or F2, or via the Module menu)
- File paths are converted to native separators when shown in dialog boxes to the user
- The settings dialog is now created on demand. This also fixes an issue with dialog z-ordering when the main application window is switched to "Stay on Top"
- Depending on the selected module the persistence of a flight is different (single aircraft vs formation flight). In order not to mess up the persistence logic switching modules is now disabled while recording 
- In order to prevent data loss when recording a flight the following functionality is now properly disabled during recording:
  * No existing flight can be loaded (including by double-clicking onto a logbook row)
  * No user aircraft can be switched in the formation (including by double-cicking onto an aircraft in the formation table)
  * Creating a new logbook or opening an existing one is disabled

### Under The Hood
- Introduction of the GeographicLib
  * Calculating undulation values
  * Geodesic calculations (distance, azimuth, relative position)
- Refactored the build system ("modern CMake")
  * Resources like the undulation file can be auomatically (optionally) downloaded at build configuration time (refer to the [BUILD.d](BUILD.md) instructions for details
  * Third-party libraries (git submodules) like the GeographicLib are automatically checked out
- Model classes refactoring
  * Template based, in order to reduce repetitive code
  * Qt Signas and slots re-wiring
- Plugin management refactoring
  * Plugin managers are now in a common PLuginManager library
  * Plugins are now built after the main application
    - Especially on macOS this allows to place the plugins inside the previously built application bundle
- And finally... on macOS the application icon set is now automatically generated at build time

## 0.10.0

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

### New Features

- IGC export plugin
  * Export flight path (position and GNSS altitude)
  * Export waypoints as flight tasks ("C records")
  * Export pilot names
- GPX import plugin
  * The GPX format contains three elements: waypoints, routes and tracks
    - Waypoints are typically "places to visit" along the way
    - Routes describe "how to get there"
    - Tracks contain the actually travelled path (typically recorded with a GPS device)
  * The import dialog allows to select which elements to use for flight waypoints and the actual flown path
  * It also offers a default altitude and velocity, as timestamps and elevation values are optional values in the GPX format
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
  * The initial heading is now calculated on the "first significant movement", compensating GPS inaccuracies (when the glider is standing still on the ground)
  * Date and time of turn points (waypoints) defined by the task list ("C records") are now updated with the actual flight time, based on the closest flown position
  * Environmental noise level (ENL) determines the engine state (on/off, propeller lever)
- Logbook
  * The "Delete" key now deletes the selected flight
  * Columns will only be resized to content upon first data load; afterwards manual column resizing won't be changed
- Replay
  * A replay loop button has been added: when enabled the replay restarts at the beginning whenever the end is reached
  * In the settings the seek interval is now settable with a spinbox instead of a text field
- Recording
  * The simulation variable "indicated airspeed" is now also recorded (however only for analytical purposes - e.g. exported as "pressure altitude" in the new IGC export plugin)
  
### Bug Fixes

- The spoiler position value is now properly shown as percent (instead of "raw number") in the simulation variables dialog
  * For clarity the field is now also labeled "spoiler position" (instead of just "spoiler")
- Switched internal leading/trailing flaps position unit from "percent" to "position"
  * The greater precision (32767 vs 255 possible values) gets rid of the "flaps sound stutter" (e.g. audible in the cockpit of the A320neo)
  
### Miscellaneous

- Sky Dolly wants and promotes peace

## 0.8.6

### New Features

- IGC import plugin
  * The International Gliding Commission (IGC) approved flight recorder format for gliders
  * Basic header data ("H records") import (pilot names, glider type and ID)
  * Basic position ("B records (fix)") import
  * Basic task ("C records") import (waypoints)

### Bug Fixes

- Typo: plural of aircraft is aircraft (credits: Stefano Caporelli, Youtube comment [Sky Dolly - Formation Flying](https://www.youtube.com/watch?v=Op_zTfM3-HQ&lc=UgyZXpfBxZ5iLpqnd5F4AaABAg))
- KML import plugin
  * "Flight augmentation" (adding flight procedures, aircraft attitude etc.) does properly terminate when imported flight duration is less than 3 minutes
  * Validate the number of imported positions
  * Show a simple error message in case of import failure

## 0.8.5

### New Features

- KML import plugin which imports KML flight data from [flightaware.com](https://flightaware.com)

### Bug Fixes

- Dates and times are now properly displayed using the system locale (e.g. "8/5/2021" or "5.8.2021" for dates like "August 5 2021",
  "5:35 PM" or "17:35" for times)
  * Date and time table items are now also properly sorted, regardless of their "locale representation"
- Logbook: the date column of the "date chooser widget" is now properly resized according to its content (month names)

## 0.8.4

### Bug Fixes

- The initial velocity when recording a new formation aircraft is now properly set (indicated airspeed, as opposed to true airspeed)
  * Note that the conversion from true to indicated airspeed is currently done with a "rule of thumb" only
  * https://www.pilotmall.com/blogs/news/how-to-calculate-true-airspeed-and-what-it-is-guide
  * Depending on the altitude and especially pressure there might still be a substantial difference to the actual indicated airspeed
- Export
  * Illegal file path characters are replaced by _ in the initially suggested file path
  * KML export: exported character data (title, description) is properly enclosed in a CDATA tag
- Properly update AI aircraft when deleting the user aircraft during replay

## 0.8.3

### Bug Fixes

- The main window is now made a "parent" of the KML export dialog, making sure that the export dialog is always centered and on top of the main window
  * Especially when the "Stay on Top" option is enabled for the main window
- The initial velocity in "fly with formation" replay mode is now properly set
  * Properly converted from feet/s to knots
  * Properly converted from true to indicated airspeed (the SIMCONNECT_DATA_INITPOSITION structure really seems to expected indicated airspeed)

## 0.8.2

### Bug Fixes

- Starting the recording of the first aircraft directly in the Formation module would "teleport" the aircraft to be recorded right to the North Pole (0/0 longitude/latitude)
  * The "relative initial recording position" calculation now properly checks the returned "null position" value

## 0.8.1

### Bug Fixes

- Creating new logbooks works again
  * an attempt was made to create a backup before migration of a non-existing database

## 0.8.0

### New Features

- Plugin architecture
  * Import CSV plugin
    - Aircraft can now be _added_ to existing flights ("multiplayer formation flying")
    - Select the aircraft type (used when spawning AI aircraft in formation flights)
    - Aircraft types (name, number of engines, wingspan, ...) are constantly updated upon flight recordings
      * The default aircraft of MSFS are already in the logbook and hence immediatelly selectable when importing a CSV file
    - Also refer to new "time offset" feature in the Formation module which is useful to "synchronise multiplayer formation flights", see below
  * Export CSV plugin
  * Export KML plugin ("Google Earth")
    - Basic line style settings
    - Plugin settings are persisted in Sky Dolly application settings
  * SkyConnect plugin architecture
    - SimConnect plugin (currently FS2020 only)
    - Auto-detection of installed flight simulator (automatic plugin selection)
    - Plugin changeable at runtime via settings dialog
  
#### Modules

- Logbook
  * Filtering of flights 
    - By date via creation date tree widget
    - Search field which matches the flight title, user aircraft and destinations
    - Engine type (jet, piston, turboprop, ...)
    - Formation flights (flights having more than one aircraft)
  * The selected _user aircraft_ is now shown in the flight log table (previously: simply the first recorded aircraft of the formation was shown)
- Formation
  * A time offset can be added to the selected aircraft, effectively "shifting" it back or forth on the timeline
    - This is useful for "multiplayer formation flights" in order to synchronise the imported aircraft (also see CSV import plugin above)
  * A new "Fly with Formation" replay mode has been added:
    - All previously recorded aircraft are replayed
    - You fly with your currently loaded user aircraft with the formation ("in addition" to the recorded aircraft)
  * "Take control of recorded user aircraft" still exists
    - You can now switch control between user aircraft (also during replay), by double-clicking on the aircraft in the "Formation" table (or push the "Set User Aircraft" button)
  * Pause will now also "freeze" the user aircraft
    - For both "Fly with formation" and "Take control" replay modes
    - Additionally in "Fly with formation" replay mode user aircraft can be re-positioned relative to the _recorded_ user aircraft in the formation while paused, by clicking on one of the "relative positions"
  * Aircraft can now also be deleted from a formation flight during (paused) replay

### Improvements

- Better support for aircraft engine start & shutdown
- Additional simulation variables
  - Combustion
  - Smoke enabled/disabled (at the time of this writing not yet supported by FS2020)
- Database performance improvements:
  * "forward queries"
  * "Normalisation" of "aircraft type" data (new table - less storage space required)
- A new "View" menu has been added, providing options to show/hide various UI elements such as the module selector
- Logbook backup can now be scheduled, the logbook settings are now in the "File" menu
  * Once a month
  * Once a week
  * Daily
  * Never or whenever exiting Sky Dolly
- Older logbooks are not backed up before migration
  * This can be disabled in the new Logbook Settings (File/Logbook Settings...)
- User interface and usability refinments
  * "Active" icons
  * Small layout optimisations
  * Creating a new logbook now changes into parent directory of the actual logbook directory, when logbook file name (without extension) corresponds with the parent directory name
    - E.g. when creating a new logbook, when the existing one is "c:\Users\The User\Documents\Sky Dolly\Sky Dolly.sdlog", the file dialog will now start with "c:\Users\The User\Documents\"
    - (Previously: "c:\Users\The User\Documents\Sky Dolly\")
  * The replay speed group can now be hidden has well, just like the module selector
    - Both view options are now in the new View menu
    - Hiding the replay speed group and then switching into "minimal UI" mode (key M) allows for a truly minimalistic UI now
    - Note that the module selector and replay speed group view visibiliites cannot be toggled while in "minimal UI": switch to normal UI (key M) first
- Sky Dolly logbooks (*.sdlog) can now be associated with the Sky Dolly application: they will be opened upon double-click on the logbook (file)
  * Associating *.sdlog files with SkyDolly.exe needs to be manually done for now (right-click on file, "Open with...", choose SkyDolly.exe)
- Pressing the same shortcut for the Show Flight|Simulation Variables|Statistics (keys F, V and S respectively) now closes the corresonding dialog again

### Bug Fixes

- Proper "HiDPI support" on Windows

## 0.7.1

### Bug Fixes

- Only the database version is now read from the metadata table when comparing versions
  * Related metadata tables and columns may change in future versions, resulting in an SQL error when trying to read the entire metadata table

## 0.7.0

### New Features

- New module: Formation - record an unlimited number of aircraft and replay them altogher, with one single user interface
  * Simply add new aircraft to the formation by clicking the "record" button while in the Formation module (the plus (+) sign inidcates that aircraft are being added)
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

- Make sure that the "delete flight" confirmation dialog is on top of the main window (specifically when the "stay on top" option is enabled)

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
- Decimal points in logbook file names are not "swallowed" anymore (e.g. "My Logbook v0.6.sdlog" is a perfectly fine filename)
- UI elements such as Record and CSV Import/Export are now properly enabled or disabled based on the logbook (database) connection status

## 0.6.1

### Bug Fixes

- Fix "logboog" typo in preview message
- Improve minimal UI switching
- Don't reset the elapsed time to 0 when unpausing a recording
- Keep displayed time when pausing a replay (don't set to total replay time)
- Automatically connect with simulator when seeking position
- Also pause the initially loaded flight
- When "repeat values" is enabled then also repeat values with "zero value" (e.g. "canopy closed")
- Automatically reconnect in case the SimConnect server (flight simulator) has crashed
- Also store the flight into the logbook if a paused recording is stopped
- Don't request SIMCONNECT_PERIOD_NEVER twice (e.g. when stopping a paused recording): prevent SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID server exception
- Ensure that waypoints have distinct timestamp (prevent database unique constraint violation)

## 0.6.0

### New Features

- Add SQLite database support ("logbook") for persistence
  * Each flight is automatically persisted
  * Simple load / delete functionality
  * DB backup and optimisation functionality
- Simple logbook table which lists the recorded flights
- Each flight has an editable title and description
- The previous "Scenario" is now called a "Flight" ("Flight" dialog, shortcut key: F)
- Additional simulation variables support
  * Waypoints (ICAO, latitude/longitude/altitude) are shown in the order of approach; each waypoint is only counted once
  * Local and zulu simulation time
- Replay speed now also available in replay menu, including shortcuts: CTRL + [1 - 4] for slow motion, SHIFT + [1 - 5] for timelapse
- The custom replay speed factor can now be entered either as an absolute factor (1.0 = normal speed) or as percent (100% = normal speed)
- Remove "Yoke X|Y Position" simulation variables (not needed: yokes are still properly animated)
- Option to enable a "minimal UI" (shortcut key: M)
- The window geometry and state are persisted and restored upon application start

## 0.5.2

### New Features

- New "repeat values" settings for selected simulation variables have been introduced: canopy and flaps  
  * Certain aircraft counteract the requested simulation variables, e.g. the canopy closes or the flaps retract automatically
  * So with the "repeat" option enabled and if the given simulation variable has a value greater zero (e.g. "canopy open") then its value is repeatedly sent to Flight Simulator
  * The optimal settings are to disable the "repeat" options (in which case simulation variables are only sent when the value actually changes)

### Bug Fixes

- Fixes a crash which would occur when quitting the application and when the simulation variables dialog was still open
- Altitude and other simulation variables measured in feet are now properly displayed with their unit ('ft')
- Replaced the non-writeable FOLDING WING HANDLE POSITION simulation variable with FOLDING WING LEFT|RIGHT PERCENT

## 0.5.1

### Bug Fixes

- Properly parse the custom speed factor according to the current system locale (decimal delimiter may be , or .)
- The initial custom replay speed factor is now displayed with decimal places (e.g. 1.00 or 1,00 - to illustrate the expected decimal delimiter according to the system locale)
- Custom speed factor tooltip shows the valid value range
- Properly name simulation variable "Yoke X Position" (instead of Y)
- Simulation variables are now formatted according to the system locale

## 0.5.0

### New Features
- Simulation variables have been split into distinct requests, saving both on required space and CPU usage
- The replay speed is now a factor instead of percentage value (1 = normal speed, 2 = double speed etc.)
- Additional simulation variables: 
  * Cowl flaps
  * Electrical master battery
  * General engine starter
  * Folding wing handle
- Simulation variables now shown with units (feet, hPa, ...)
- Added a new "Flight" dialog, showing information about the aircraft and initial flight conditions
- The last CSV export/import directory is stored
- A basic error dialog shows when an import/export error occurs
- The seek interval can now be defined in the settings (either absolute in seconds, or in percent of the record duration)

## 0.4.2

### Bug Fixes

- Do not start the elapsed timer when seeking a play position and not in replay mode (replay does not "skip ahead" when started)

## 0.4.1

### Bug Fixes

- The time widget is now editable when replay is paused
- After importing a CSV file only pause when in connected state
- Do not start the elapsed timer when skipping to begin of recording
- Properly hide window when stay on top is deselected

## 0.4.0

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
- When recording sample rate is set to 1 Hz ("one sample per second") then the SimConnect request period is now also set to 1 Hz
- A new "auto" recording sample rate has been added 
  * Recording: samples are collected in an event-based manner ("as they arrive")
  * Replay: samples are always sent "per visual frame"
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
- "Show simulation variables" action is now deselected when the dialog is closed (via close button)
- Set the initial start position ("on ground") when replay from (or seek to) start
- Prevent aircraft from "dropping out of the sky" after replay, by recording body velocities

## 0.3.1

### Bug Fixes

- CSV import: Properly handle platform-specific line endings 

## 0.3.0

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

### New Features

- Recording & replay of basic flight controls (rudder, ailerons, flaps, gear, ...)
- Updated user interface
- Separate simulation variables dialog
- Simulation variables are now also shown during replay (in the new simulation variables dialog)
- Add record/play/pause actions, with keyboard shortcuts (r = record, space = play, p = pause)

### Bug Fixes

- Validate the recorded simulation variables, specifically the position (sending invalid values back freezes or even crashes MSFS)
- Properly store the already elapsed time when changing the replay speed, for a smooth continuation of the replay

## 0.1.0

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
