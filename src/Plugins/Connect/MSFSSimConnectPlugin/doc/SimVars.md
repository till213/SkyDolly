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

- XAnimation: animation simulation variables that are exclusively sent to AI aircraft, for animation purposes (but no effect on user aircraft, e.g. "rudder deflection" et al.)
- XCommon: common simulation variables that are sent to both user- and AI aircraft as simulation variables
- XCore: core simulation variables might be sent as either simulation variables or events exclusively to the user aircraft (not to AI aircraft)
- XCoreEvent: core events are sent exclusively to the user aircraft (not to AI aircraft), e.g. "gear stear angle pct"
- XEvent: event variables are sent as event to the user aircraft and (possibly) as simulation variables to AI aircraft
- XInfo: simulation variables that are only stored for information purposes or that are sent exclusively to the user aircraft as events

## Request and Response Records
The following "cumulative records" are actually registered with SimConnect as data definitions (also refer to SimConnectType.h):

### Response (Recording)
- XAll: the combination of all above simulation variables (XCommon + XCore + XEvent)

### Request (Replay)
- XUser: the combination of XCommon and possibly XCore; data sent as simulation variables to the user aircraft
- XAi: the combination of XCommon and possibly XEvent; data sent as simulation variables to AI aircraft
