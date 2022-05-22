# Release Checklist

## Preflight

- Ensure that proper project version has been set in the main [CMakeLists.txt](CMakeLists.txt)
  * E.g. `VERSION 0.11.0`
- Ensure that the final migration step in [migr.sql](src/Persistence/src/Dao/SQLite/Migration/migr.sql) sets the proper version
- Choose and set a suitable "version codename" in [Version.cpp](src/Kernel/src/Version.cpp)

## Take-Off

## Postflight
