
# Release Checklist

## Preflight
### Version info
- Ensure that the proper project version has been set in the main [CMakeLists.txt](CMakeLists.txt)
  * E.g. `VERSION 0.11.0`
- Ensure that the copyright year in [SkyDolly.rc.in](res/SkyDolly.rc.in) and (Info.plist.in)[res/Info.plist.in] is up to date

- Choose and set a suitable "version codename" in [Version.cpp](src/Kernel/src/Version.cpp)
- Update the "preview dialog" message in [MainWindow.cpp](src/UserInterface/src/MainWindow.cpp)
- Adjust the `PreviewInfoDialogBase` in [Settings.cpp](src/Kernel/src/Settings.cpp) in order to show the preview dialog three times
- Adjust the _Latest (pre-)release: Sky Dolly 0.10.0_ in [README.md](README.md)

### Git
- Push all changes
- Merge all feature / bug fix branches into main  (https://github.com/till213/SkyDolly/pulls)
- Pull all changes
- Switch to main branch
- Check integration actions (https://github.com/till213/SkyDolly/actions)

## Taxiing
### Build
- Select "Release"
- Ensure that the CMake option `SKY_FETCH_EGM` is enabled
- Run CMake
- Build
- Repeat on macOS and Linux

### Package
- Create a new package folder, e.g. `SkyDolly-v0.11.0`
- Copy all executables and libraries from the build `bin` folder into the new package folder (e.g. `SkyDolly-v0.11.0`)
- Ensure that the following dependencies are in the package folder:
  * platforms/qwindows.dll
  * sqldrivers/sqlite.dll
  * styles/qwindowsvistastyle.dll
  * libgcc_s_seh-1.dll, libstdc++-6.dll, libwinpthread-1.dll
  * Qt5Widgets.dll, Qt5Core.dll, Qt5Gui.dll, Qt5Sql.dll
  * SimConnect.dll
  * Geographic.dll
- Ensure that the following dependencies are removed from the package folder:
  * Plugins/Connect/PathCreator.dll
  * SkyMathTest.exe, SkySearchTest.exe, ...
- Ensure that the resources files are in the package folder:
  * Resources/geoids/egm2008-5.wld, egm2008-5.pgm. egm2008-5.pgm.aux.xml

### Smoke Test
- Launch SkyDolly.exe
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
- Export as CSV
- Import same file as CSV
- Enable / disable minimal UI (key **M**)
- Open a Sky Dolly v0.6 logbook (create a copy first)

## Take-Off
- ZIP the package folder (e.g. `SkyDolly-v0.11.0.zip`)

### GitHub
- Create a new release (https://github.com/till213/SkyDolly/releases/new)
- Choose a tag, e.g. `v0.11.0`
- Ensure that _Target_ is set to `main`
- Choose a title, e.g. `Sky Dolly 0.11.0`
- Provide a description (summary of the changelog)
- Attach the ZIP archive (e.g. `SkyDolly-v0.11.0.zip`)
- Check the _This is a pre-release_ option (as appropriate)

Description template:

> This release provides feature X.
> * A new feature Y
> * Another feature Z
>
> Bug fixes:
> * Bug fix A
> * Bug fix B

- Publish

## Climb
- Fetch the new tag: `git pull`
- Create a bug fix branch, e.g.: `git branch 0.11.0`
- Bump the version to e.g. `0.11.1` in the main [CMakeLists.txt](CMakeLists.txt)
- Commit, e.g. `git commit -m "Bump version to 0.11.1"`
- Push, e.g. `git push --set-upstream origin 0.11.0`
- Switch back to _main_: `git checkout main`
- Bump the version to e.g. `0.12.0` in the main [CMakeLists.txt](CMakeLists.txt)
- Commit, e.g. `git commit -m "Bump version to 0.11.1"`
- Update the shasum in (SHASUM256.md)[SHASUM256.md]

## Postflight

