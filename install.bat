@echo off

setlocal

:: checking for args
if "%~1"=="global" (
    set installtype="u"
) else if "%~1"=="user" (
    set installtype="g"
) else (
    echo Please specify if the program should be installed locally: 
    echo install user
    echo or globally:
    echo install global
    exit /b
)

echo Installing sortetdir...

if "%installtype%"=="u" (
    echo Creating a folder in the local appdata and moving config...
    if not exist "%localappdata%\sortetdir" mkdir "%localappdata%\sortetdir"
    if exist  "config.json" (
       move "config.json" "%localappdata%\sortetdir"
    ) else (
       echo "config.json" could not be found in the current directory.
    )
    
) else (
    echo Creating a folder in the appdata and moving config...
    if not exist "%appdata%\sortetdir" mkdir "%appdata%\sortetdir"()

    if not exist "%appdata%\sortetdir\config.json" (
          if exist  "config.json" (
           move "config.json" "%appdata%\sortetdir"
        ) else (
           echo "config.json" could not be found in the current directory. A new, EMPTY, config will be created.
           pause
           echo {} > "%appdata%\sortetdir\config.json"
           move "config.json" "%appdata%\sortetdir"
        )
    ) else (
      echo config.json already exists in %appdata%\sortetdir, nothing was changed.
    )
    
    
)


echo "Compiling program..."
make cwin




if "%installtype%"=="u" (
    set /p ADDPATH="Should %cd%\bin be added to the USER PATH?"
    
) else (
    set /p ADDPATH="Should %cd%\bin be added to PATH? (y/n): "

    if "%ADDPATH%"=="y" (
        echo Adding .bin to PATH

        if "%%"=="" (
           
        ) else (
           
        )
        set %PATH% %PATH%;%cd%\bin
    ) else (
        echo %cd% was not added to PATH
    )

    
)

endlocal