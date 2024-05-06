# Aircraft

The selectable aircraft are stored in table aircraft_type. Default aircraft, that is all aircraft of the MSFS Deluxe version and freely available aircraft by Asobo - are inserted at migration time, using the LogbookMigration.sql source.

## Adding new default aircraft

* Start Sky Dolly, create a new logbook
* In MSFS, enable Developer Mode
* Spawn at any airport (anyhwere)
* Open the Aircraft Selector dialog
* Select the aircraft to be added
* In Sky Dolly, quickly record a flight
* Repeat

Then

* Open the logbook with DB Browser for SQLite
* File | Export | Database to SQL file...
* Only select the aircraft_type table
  - Keep column names in INSERT INTO: OFF
  - Multiple rows (VALUES) per INSERT statement: ON
  - Export everything
  - Keep old schema
* Open the exported file (e.g. aircraft_type.sql, with Visual Studio Code) and...
* ... copy and adapt an existing aircraft type migration in LogbookMigration.sql with the content of aircraft_type.sql
