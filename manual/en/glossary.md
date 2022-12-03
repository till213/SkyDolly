# Glossary

[Index](index.md) \| [Glossary](glossary.md)

This chapter explains terms which occur in the context of Sky Dolly and this manual.

Term             | Description
-----------------|------------
AI Aircraft      | The → *flight simulator* typically populates the sky and runways with various aircraft, also called *AI aircraft* (AI stands for artificial intelligence). An AI aircraft may have a predefined flight plan ("fly from A to B by going over C"), is following some real-world flight or - in the context of a → *formation flight* - is created and controlled by Sky Dolly.
Aircraft         | In Sky Dolly the *aircraft* represents the recorded data, including position data and static data such as aircraft type, tail number and category. Each aircraft is associated with a → *flight*.
API              | API stands for application programming interface and is mostly relevant for developers (only). However this term frequently appears in forum discussions in combination with → *SimConnect*. It represents the "public interface" - the contract between the function provider and caller, broadly put - that a caller can use to access the funtionality and data.
Flight           | A *flight* stores the initial flight conditions (visibility, temperature, wind direction, ...) and also the flight plan of its *first* recorded → *aircraft*. A flight may have several associated aircraft (→ *formation flight*).
Flight simulator | The *flight simulator* acts as the "server" in the communication with Sky Dolly ("client"), which happens with the official → *SimConnect* → *API*. Sky Dolly is built upon a → *plugin* architecture, and as such also the connection with the *flight simulator* is plugin-based. Currently MSFS 2020 is supported as *flight simulator*.
Formation flight | A → *flight* with more than one *aircraft*. A *formation flight* has exactly one → *user aircraft*, all the other aircraft are spawned as "AI controlled aircraft" (→ *AI aircraft*).
GNSS             | Global Navigation Satellite System. Used to determine the position and altitude, the most well-known system probably being the → *GPS*.
GPS              | Global Positioning System, a satellite based sytem to determine the position and altitude.
Position         | The *position* of the *aircraft* on the ground or in the sky, specified by its latitude, longitude and altitude (in feet). Each → *aircraft* has its own list of *positions*, essentially defining its flight path.
Waypoint         | A point of the flight plan. *Waypoints* are stored for each → *flight*, according to the flight plan of the *first* recorded → *aircraft*.
SimConnect       | *SimConnect* is the name of the official → *API* with which add-ons such as external applications like Sky Dolly can communicate with the → *flight simulator*. *SimConnect* first saw its use in Flight Simulator X, but is nowadays supported by a range of other → *flight simulators*, including MSFS 2020.
User aircraft    | The *user aircraft* is the one being followed by the camera in the → *flight simulator*, or in other words: the one which the user normally has control over.

[Index](index.md) \| [Glossary](basic.md)