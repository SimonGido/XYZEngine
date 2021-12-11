@echo off
pushd %~dp0\..\
call MSBuild.exe XYZEngine.sln
popd
PAUSE