# Changelog

## 0.8.0

### New Features

- Plugin architecture
  * Import CSV plugin
    - Aircrafts can now be _added_ to existing flights ("multiplayer formation flying")
    - Select the aircraft type (used when spawning AI aircrafts in formation flights)
    - Aircraft types (name, number of engines, wingspan, ...) are constantly updated upon flight recordings
      * The default aircrafts of FS 2020 are already in the logbook and hence immediatelly selectable when importing a CSV file
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
    - This is useful for "multiplayer formation flights" in order to synchronise the imported aircrafts (also see CSV import plugin above)
  * A new "Fly with Formation" replay mode has been added:
    - All previously recorded aircrafts are replayed
    - You fly with your currently loaded user aircraft with the formation ("in addition" to the recorded aircrafts)
  * "Take control of recorded user aircraft" still exists
    - You can now switch control between user aircrafts (also during replay), by double-clicking on the aircraft in the "Formation" table (or push the "Set User Aircraft" button)
  * Pause will now also "freeze" the user aircraft
    - For both "Fly with formation" and "Take control" replay modes
    - Additionally in "Fly with formation" replay mode user aircraft can be re-positioned relative to the _recorded_ user aircraft in the formation while paused, by clicking on one of the "relative positions"
  * Aircrafts can now also be deleted from a formation flight during (paused) replay

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

## 0.7.0

### New Features

- New module: Formation - record an unlimited number of aircrafts and replay them altogher, with one single user interface
  * Simply add new aircrafts to the formation by clicking the "record" button while in the Formation module (the plus (+) sign inidcates that aircrafts are being added)
  * Previously recorded aircrafts are replayed during recording of the new aircraft
  * Change the user aircraft (the one which is being followed by the camera) at any time during replay
  * Take control over the user aircraft any time during replay: manually fly it at any time (and give control back to replay at any time)
  * Delete aircrafts from a formation
  * Set the position (relative to the current user aircraft) for the new aircraft to be recorded
- All aircrafts are automatically persisted and added to the current flight
- The logbook now shows the number of aircrafts per flight
- Start a new flight (starting with a single aircraft) by switching into the Logbook module
- The existing Simulation Variables and Flight dialogs show data about the currently selected user aircraft
- The module selector may be hidden
- Reduced data size of requested simulation variables

### Bug Fixes

- Make sure that the "delete flight" confirmation dialog is on top of the main window (specifically when the "stay on top" option is enabled)

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
  * Certain aircrafts counteract the requested simulation variables, e.g. the canopy closes or the flaps retract automatically
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
- Prevent aircrafts from "dropping out of the sky" after replay, by recording body velocities

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

- Make sure that the first sampled data properly starst with a timestamp of 0, in order to properly interpolate the requested values
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

- Validate the recorded simulation variables, specifically the position (sending invalid values back freezes or even crashes FS 2020)
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
