"%MSFS_SDK%\Tools\bin\fspackagetool.exe" package.xml
REM This is the community folder path for the Steam version of MSFS
xcopy /E /Y /I .\Packages\till213-panel-skydolly "%USERPROFILE%\AppData\Roaming\Microsoft Flight Simulator\Packages\Community\till213-panel-skydolly\"
