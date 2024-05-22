
# Release Checklist

## Preflight
### Version info
- Ensure that the proper project version has been set in the main [CMakeLists.txt](CMakeLists.txt)
  * E.g. `VERSION 0.16.0`
- Ensure that either the [logbook migration](src/Persistence/src/Dao/SQLite/migr/LogbookMigration.sql) and/or the [location migration](src/Persistence/src/Dao/SQLite/migr/LocationMigration.sql) updates the database metadata with the current version (with _different_ migration UUIDs)
- Ensure that the copyright year in [SkyDolly.rc.in](src/SkyDolly/res/SkyDolly.rc.in) and [Info.plist.in](src/SkyDolly/res/Info.plist.in) is up to date
- Choose and set a suitable "version codename" in [Version.cpp](src/Kernel/src/Version.cpp)
- Update the "preview dialog" message in [MainWindow.cpp](src/UserInterface/src/MainWindow.cpp)
- Adjust the `PreviewInfoDialogBase` in [Settings.cpp](src/Kernel/src/Settings.cpp) in order to show the preview dialog three times
- Adjust the _Latest (pre-)release: Sky Dolly 0.16.0_ in [README.md](README.md)
- Adjust the supported versions in [SECURITY.md](SECURITY.md)

### Settings
- Ensure that settings are converted to latest version in [SettingsConverter.cpp](src/Kernel/src/SettingsConverter.cpp)

### Documentation
- Ensure that the [example SQL](doc/SQL) still works

### Git
- Push all changes
- Merge all feature / bug fix branches into main (https://github.com/till213/SkyDolly/pulls)
- Pull all changes
- Switch to main branch
- Check integration actions (https://github.com/till213/SkyDolly/actions)

## Taxiing
### Build
- Select "Release"
- Ensure that the CMake option `SKY_TESTS` is enabled
- Ensure that the CMake option `SKY_FETCH_EGM` is enabled
- Run CMake
- Build
- Run all tests
- Repeat on macOS and Linux

### Package
- Create a new package folder, e.g. simply `SkyDolly`
- Copy all executables and libraries from the build `bin` folder into the new package folder (`SkyDolly`)
- Ensure that the following dependencies are in the package folder:
  * platforms/qwindows.dll
  * sqldrivers/sqlite.dll
  * styles/qmodernwindowsstyle.dll
  * libgcc_s_seh-1.dll, libstdc++-6.dll, libwinpthread-1.dll
  * Qt6Core.dll, Qt6Gui.dll, Qt6Sql.dll, Qt6Widgets.dll 
  * SimConnect.dll
  * GeographicLib.dll
- Ensure that the following dependencies are removed from the package folder:
  * Plugins/Connect/PathCreator.dll
  * Plugins/Module/Template.dll
  * SkyMathTest.exe, SkySearchTest.exe, ...
- Ensure that the resources files are in the package folder:
  * Resources/geoids/egm2008-5.wld, egm2008-5.pgm. egm2008-5.pgm.aux.xml
  * Resources/migr/Locations.csv

### Smoke Test
- Launch `SkyDolly.exe`
- Ensure that the Help / About... information is as expected
- Compare the version git hash code with the one of the current `main` branch (`git rev-parse --short HEAD`)
- Ensure that the connect and import/export plugins are present
- Launch the flight simulator
- Record a new flight
- Record a new aircraft, with replay mode set to
  * Normal
  * Take control of recorded user aircraft
  * Fly with formation
- Repeat with _Set relative position_ disabled
- Delete aircraft, change user aircraft in formation, with  _Set relative position_ enabled and disabled
- Change bearing and distance, with  _Set relative position_ enabled and disabled
- Export as Logbook
- Import same file as Logbook
- Enable / disable minimal UI (key **M**)
- _Open_ a Sky Dolly v0.6 logbook (create a copy first)
- _Import_ a Sky Dolly v0.6 logbook (create a copy first)

## Take-Off
- ZIP the package folder (e.g. `SkyDolly-v0.17.0.zip`)

## After Take-Off Check
- Upload the ZIP archive to [VirusTotal](https://www.virustotal.com/)

### GitHub
- Create a new release (https://github.com/till213/SkyDolly/releases/new)
- Choose a tag, e.g. `v0.17.0`
- Ensure that _Target_ is set to `main`
- Choose a title, e.g. `Sky Dolly 0.17.0`
- Provide a description (summary of the changelog)
- Attach the ZIP archive (e.g. `SkyDolly-v0.17.0.zip`)
- Check the _This is a pre-release_ option (as appropriate)

Description template:

> This release provides feature X.

> New features:
> - A new feature Y
> - Another feature Z
>
> Improvements:
> - Improvement 1
> - Improvement 2
>
> Bug fixes:
> - Bug fix A
> - Bug fix B

- Publish

## Climb
- Fetch the new tag: `git pull`
- Create a bug fix branch, e.g.: `git branch 0.16.0`, `git checkout 0.16.0`
- Bump the version to e.g. `0.16.1` in the main [CMakeLists.txt](CMakeLists.txt)
- Commit, e.g. `git commit -m "Bump version to 0.16.1"`
- Push, e.g. `git push --set-upstream origin 0.16.0`
- Switch back to _main_: `git checkout main`
- Update the shasum in [SHASUM256.md](SHASUM256.md)

## Cruise

### Flightsim.to
- Publish the release on: https://flightsim.to/file/9067/sky-dolly
- Update screenshots

## Landing

### NOTAM
- Announcement on [flightsim.to Sky Dolly group](https://flightsim.to/group/9067/sky-dolly)
- Announcement on [MSFS Product Announcements](https://forums.flightsimulator.com/t/update-0-10-sky-dolly-flight-recorder-with-formation-replay-logbook-import-export-till213/497107/)
- Announcement on [AVSIM general freeware](https://www.avsim.com/forums/topic/600337-sky-dolly-free-flight-recorder-replay-app-fs2020/)
- Announcement on [Buy me a coffee](https://www.buymeacoffee.com/)
