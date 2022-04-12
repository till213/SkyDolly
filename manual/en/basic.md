# Basic Usage

[Index](index.md) \| [Basic Usage](basic.md)

This chapter describes the basic usage of Sky Dolly, specifically recording and replaying a flight.

## Connection With The Flight Simulator
Sky Dolly communicates with the *flight simulator* via the SimConnect API, an application programming interface which allows external applications such as Sky Dolly (the *client* application) to communicate with the *flight simulator* (the *server* application).

While certain functionality like managing the *logbook* or exporting a *flight* does not require an active connection recording and replaying a *flight* certainly does. The good news: Sky Dolly automatically handles the connection with the *flight simulator*, as soon and when such a connection is required, for instance when pressing the *record* button. However the requirement is that the *flight simulator* is already running at that point and accepting such connection requests.

> For all practical matters it also means that an *active* flight must be in progress. For MSFS 2020 this means that you have selected a departure on the world map, clicked the yellow "READY TO FLY" button and are essentially sitting in the cockpit, ready to start your engines or ready for take-off.

## Recording a Flight
The following steps will guide you through the process of recording a flight and monitoring its progress in Sky Dolly.

At any time an *active* flight is in progress in the *flight simulator*, switch to the Sky Dolly windows and:

- Click the *Record* button (or press key **R**)
- Switch back to the *flight simulator* windows and perform your flight
- Once done with the flight switch again to the Sky Dolly window and stop the recording by pressing on the *Stop* button (or press the **ESC** key)

Alternatively in order to stop the recording you may also press the *Record* button again (or key **R**).

That is it! You have successfully recorded a *flight* with Sky Dolly. Note that at the point the recording is stopped the *flight* (including the recorded *aircraft*) is automatically stored in the *logbook*, which will be described shorty.

> So what did we record after all? Sky Dolly does neither record a video stream, nor does it record audio from the *flight simulator*. Instead the most important aircraft state variables - the so-called *simulation variables* - such as position, velocity and engine state (e.g. the thrust lever position) are recorded. During replay those state variables are then sent back to the *flight simulator,* which then reconstructs the flight.

## The Logbook
If you have just recorded a *flight* then it is already in memory and ready for replay. But if you just have launched Sky Dolly then no *flight* is loaded at this point.

In order to load a flight:

- Make sure you have activated the *logbook* module, by clicking on the *Logbook* button (or press key **F1**)
- Alternatively activate the *logbook* module via the *Module* menu, by selecting the *Logbook* menu entry
- Select a *flight* in the *logbook* table
- Click the *Load* button
- Alternatively you may double-click a row in the table to load it

> If you double-click on a table cell in the *Title* column you will go into edit mode and you can change the title (e.g. "My first flight"). Double-clicking any other cell will load the corresponding *flight* into memory, for replay (or export, also refer to chapter [Import and Export](import-export.md)).

## Replaying a Flight
Once a *flight* is loaded into memory it is ready for replay.

> It may be less obvious than in the case of *recording* a flight, but also for replay an active flight must be present in the *flight simulator*. For MSFS 2020 that means that an aircraft has been selected, a location on the worldmap has been selected and the *READY TO FLY* button has been pressed, that is we are again sitting in the cockpit and either ready to start the enginess, for take-off or we are already airborne.
>
> Note: the actual start location of the flight in the *flight simulator* is not important: upon replay start the aircraft will be "teleported" to the correct location, according to the recorded position data (latitude, longitude and altitude).

To replay the *flight*:

- Press the *Play* button (or press **space** key)
- To pause the replay press the *Pause* button (or press the key **P**)
- To resume the paused replay press the *Play* button again (or press the **space** key)
- Alternatively click the *Pause* button again (key **P**)
- You can rewind or fost-forward at any time during replay, by clicking the **Rewind** or **Fast-Forward** button (or pressing the **left arrow** or **right arrow** key)
- To stop the replay press the *Stop* button (or press the **ESC** key)
- You can also change the replay position by selecting the desired position on the timeline slider, both when then replay is started or stopped

> Make sure that crash detection is disabled in the flight simulator. Also refer to chapter [Tips and Tricks](tips-and-tricks.md).

## Monitoring Recording and Replay
There are two dialogs which provide information about what and how much is being recorded and replayed.

### Simulation Variables
The *Simulation Variables* dialog can be opened via the *Window | Simulation Variables...* menu entry or by pressing the **V** key. The dialog shows the *simulation variables* being recorded respectively replayed. The *simulation variables* are logically grouped, which is represented by the various tabs in this dialog (*Aircraft*, E*ngine, *Primary Controls*, ...).

Some value fields are rendered as *disabled* user interface elements. This indicates that the corresponding simulation variable is *inactive*, which means that it has not changed its value within a certain time window.

> Sky Dolly records only those simulation variable groups if at least one *simulation variable* in that group has changed its value. This not only allows for a memory-efficient storage, but also for the least possible CPU usage during replay, because - by implication - only those simulation variable groups are sent back to the *flight simulator* where an actual value change has occured.

All values are read-only and hence for information purposes only.

> You can also close the dialog by pressing the same key **V** again.

### Statistics
The *Statistics* dialog can be opened via the *Window | Statistics...* menu entry or by pressing the **S** key. The dialog shows basic statistis:

- The selected *recording* sample rate (in Hertz)
- The actual *recording* sample rate (in Hertz)
- The number of sampled data points
- The total duration of the *flight* (in hours, minutes, seconds)
- The size of the sampled data (in MiB, KiB, Bytes)

> You can also close the dialog by pressing the same key **S** again.

[Index](index.md) \| [Basic Usage](basic.md)
