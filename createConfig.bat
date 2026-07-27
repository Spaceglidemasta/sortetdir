@echo off
setlocal enabledelayedexpansion

echo This program will create a file at "%appdata%\sortetdir\config.json". Proceed? (y/n)
set /p i=

if /i "%i%"=="y" goto proceed
if /i "%i%"=="n" goto abort_now
echo Invalid input: "%i%"; Aborting
exit /b 1

:proceed
if not exist "%appdata%\sortetdir" (
    mkdir "%appdata%\sortetdir"
    echo Created %appdata%\sortetdir
) else (
    echo %appdata%\sortetdir already exists, skipping creation
)

if not exist "config.json" (
    echo Could not find config.json in current directory. Aborting
    exit /b 2
)

if not exist "%appdata%\sortetdir\config.json" goto do_copy

echo %appdata%\sortetdir\config.json already exists. Overwrite config? (y/n)
set /p oi=

if /i "%oi%"=="y" goto do_copy
if /i "%oi%"=="n" (
    echo Aborting
    exit /b 0
)
echo Invalid input: "%oi%"; Aborting
exit /b 1

:do_copy
copy "config.json" "%appdata%\sortetdir\"
echo Copied config.json to %appdata%\sortetdir\config.json successfully
exit /b 0

:abort_now
echo Aborting
exit /b 0