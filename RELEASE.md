# Release Checklist

## Preflight

- Ensure that the proper project version has been set in the main [CMakeLists.txt](CMakeLists.txt)
  * E.g. `VERSION 0.11.0`
- Ensure that the copyright year in [SkyDolly.rc.in](res/SkyDolly.rc.in) and (Info.plist.in)[res/Info.plist.in] is up to date
- Ensure that the final migration step in [migr.sql](src/Persistence/src/Dao/SQLite/Migration/migr.sql) sets the proper version
- Choose and set a suitable "version codename" in [Version.cpp](src/Kernel/src/Version.cpp)
- Update the "preview dialog" message in [MainWindow.cpp](src/UserInterface/src/MainWindow.cpp)
- Adjust the `PreviewInfoDialogBase` in [Settings.cpp](src/Kernel/src/Settings.cpp) in order to show the preview dialog three times

## Take-Off

## Postflight
