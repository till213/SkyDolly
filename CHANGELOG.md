# Changelog

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

## 0.2.1

### Bug Fixes

- Properly store aircraft control simulation variables, such as ailerons, yoke position, throttle, elevators and rudder

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

## 0.3.1

### Bug Fixes

- CSV import: Properly handle platform-specific line endings 

## 0.4.0

### New Features

- Additional simulation variables: propeller RPM and mixture lever positions
- Reduce required memory for sample points by over 100% (96 bytes vs previous 200 bytes per sampled data point), by using integer data types instead of double values for selected simulation variables
- Pause is enabled after CSV data import

### Bug Fixes

- Properly reset play position to begin after importing CSV data
