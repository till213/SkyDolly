# Simulation Variables

## Records
The simulation variables are organised into records, covering the following topics:

- Aircraft position
- Engine
- Primary flight controls
- Secondary flight controls
- Aircraft handles
- Light

## Topics
Each topic X is furthermore split into sub-records:

- XCommon: common simulation variables that are sent to both user- and AI aircraft
- XCore: core simulation variables might be sent as simulation variables or events to the user aircraft
- XEvent: event variables are sent as event to the user aircraft and (possibly) as simulation variables to AI aircraft

## Request and Response Records
The following "cumulative records" are actually registered with SimConnect as data definitions (also refer to SimConnectType.h):

### Response (Recording)
- XAll: the combination of all above simulation variables (XCommon + XCore + XEvent)

### Request (Replay)
- XUser: the combination of XCommon and possibly XCore; data sent as simulation variables to the user aircraft
- XAi: the combination of XCommon and possibly XEvent; data sent as simulation variables to AI aircraft

## Events
All other simulation variables, specifically those in XCore and XEvent, are sent as events to the user aircraft (only)
