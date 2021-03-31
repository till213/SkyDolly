# Changelog

## 0.5.0

### New Features
- Simulation variables have been split into distinct requests, saving both on required space and CPU usage

### Bug Fixes

- The elapsed timer is now only (re-)started when required: only for playback and recording (but e.g. not when seeking to a position while playback is stopped)

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
  * Playback: samples are always sent "per visual frame"
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
- Set the initial start position ("on ground") when playback from (or seek to) start
- Prevent aircrafts from "dropping out of the sky" after playback, by recording body velocities

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

- Recording & playback of basic flight controls (rudder, ailerons, flaps, gear, ...)
- Updated user interface
- Separate simulation variables dialog
- Simulation variables are now also shown during replay (in the new simulation variables dialog)
- Add record/play/pause actions, with keyboard shortcuts (r = record, space = play, p = pause)

### Bug Fixes

- Validate the recorded simulation variables, specifically the position (sending invalid values back freezes or even crashes FS 2020)
- Properly store the already elapsed time when changing the playback speed, for a smooth continuation of the replay

## 0.1.0

### Features

- Basic recording functionality: record, pause
- Basic aircraft data is recorded:
  * Latitude, longitude, altitude
  * Pitch, bank, heading
  * Aircraft info: on ground, model name
- Basic playback functionality: play, pause, select playback position
- Selectable playback speed

### Bug Fixes

- n/a
