# Design

## In a Nutshell
Sky Dolly is a stand-alone application which communicates via the SimConnect API with the flight simulator. During flight recording a selected set of _simulation variables_ such as latitude, longitude and altitude is received and during replay sent back to the flight simulator, which then re-creates the flight.

## Plugin Architecture
The functionality of Sky Dolly can be extended at runtime with plugins. The following functional areas are currently _plugable_:

- Connection with flight simulator
- Import
- Export

The plugins have access to the common functionality such as data interpolation, persistence and flight analytics, which is provided in modules (shared libraries).

## Persistence
All recorded flights are automatically persisted in a database, called the _logbook_. SQLite is used as database technology, a simple but efficient file-based database.

Data is read and written via data access objects (DAO), which on their turn are used by _services_: the services take care of transaction management and provide the data to the application.

## Sampling
Sky Dolly does not rely on a _fixed_ interval sampling: instead it uses a dynamic event-based sampling, or in other words: "as data _changes_ become available".

Position data (latitude, longitude and altitude, that is) is sampled at 1 Hz only: this low sampling rate acts as a low pass filter and is expected to smoothen out any frame drops ("stutter") that may have occurred during recording. All other simulation variables are sampled at each rendered visual frame ("as data becomes available"), as indicated by the simulator via event.

The _replay_ sample rate on the other hand is always event-based ("per rendered frame"), so it is essentially the flight simulator that controls the sample rate.

In any case the data is interpolated during replay. Depending on the use case various interpolation methods are applied, ranging from simple _nearest neighbour_, _linear_ to a bit more sophisticated _cubic spline_ interpolation. For instance a lever in the cockpit is expected to be moved in a "linear fashion", while the flight position may be more erratic, so cubic spline interpolation is applied for aircraft position and attitude.

Resampling may also be applied in various export plugins, in order to reduce (or even increase) the number of exported sample points.

## Design Papers
Here are some historical _design papers_ that kept piling up next to my computer. They illustrate the fact that Sky Dolly was one of the very few replay tools that got "modulo interpolation" correct from the very beginning, realising that e.g. heading values are to be interpolated "modulo 360 degrees".

Those "design papers" shall also illustrate my historic coffee consumption ;)

![Design paper 1](./img/sky-dolly-design-paper-1.jpg)

![Design paper 2](./img/sky-dolly-design-paper-2.jpg)

![Design paper 3](./img/sky-dolly-design-paper-3.jpg)

![Design paper 4](./img/sky-dolly-design-paper-4.jpg)

![Design paper 5](./img/sky-dolly-design-paper-5.jpg)

![Design paper 6](./img/sky-dolly-design-paper-6.jpg)

![Design paper 7](./img/sky-dolly-design-paper-7.jpg)

![Design paper 8](./img/sky-dolly-design-paper-8.jpg)
