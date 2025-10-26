@echo off
REM Script for deploying Qt application with SQL drivers
REM Usage: deploy-en.bat [deployment_path]

setlocal enabledelayedexpansion

set "DeployPath=deploy"
if not "%~1"=="" set "DeployPath=%~1"

set "QtPath=C:\.dev\Qt\6.10.0\msvc2022_64"
set "BuildPath=build\Desktop_Qt_6_10_0_MSVC2022_64bit-Debug"
set "ExeName=qt-car-dealership.exe"
set "ExePath=%BuildPath%\%ExeName%"

echo ========================================
echo Qt Car Dealership Deployment
echo ========================================
echo.

REM Check if executable exists
if not exist "%ExePath%" (
    echo [ERROR] Executable not found: %ExePath%
    echo Make sure the project is built.
    exit /b 1
)

REM Create deployment folder
if not exist "%DeployPath%" (
    mkdir "%DeployPath%"
    echo [OK] Created deployment folder: %DeployPath%
)

REM Copy executable
echo [1/5] Copying %ExeName%...
copy /Y "%ExePath%" "%DeployPath%\" >nul
if errorlevel 1 (
    echo [ERROR] Failed to copy executable
    exit /b 1
)
echo       Copied!

REM Copy resources
echo [2/5] Copying resources...
if exist "resources\" (
    xcopy /E /I /Y /Q "resources" "%DeployPath%\resources" >nul
    echo       Copied!
) else (
    echo       [WARNING] Resources folder not found
)

REM Run windeployqt
echo [3/5] Running windeployqt6...
"%QtPath%\bin\windeployqt6.exe" "%DeployPath%\%ExeName%" --no-translations >nul 2>&1
if errorlevel 1 (
    echo       [WARNING] windeployqt finished with error, but continuing...
) else (
    echo       Completed!
)

REM Copy SQL drivers
echo [4/5] Copying PostgreSQL SQL drivers...
if not exist "%DeployPath%\sqldrivers" mkdir "%DeployPath%\sqldrivers"

set "SqlDriversSource=%QtPath%\plugins\sqldrivers"
set "PsqlDriver=%SqlDriversSource%\qsqlpsql.dll"

if exist "%PsqlDriver%" (
    copy /Y "%PsqlDriver%" "%DeployPath%\sqldrivers\" >nul
    echo       [OK] qsqlpsql.dll
) else (
    echo       [ERROR] Driver not found: %PsqlDriver%
)

REM Copy debug version of driver
set "PsqlDriverD=%SqlDriversSource%\qsqlpsqld.dll"
if exist "%PsqlDriverD%" (
    copy /Y "%PsqlDriverD%" "%DeployPath%\sqldrivers\" >nul
    echo       [OK] qsqlpsqld.dll (debug)
)

REM Copy libpq.dll
echo [5/5] Searching for PostgreSQL libraries...
set "LibPqFound=0"

REM Check Qt bin
if exist "%QtPath%\bin\libpq.dll" (
    copy /Y "%QtPath%\bin\libpq.dll" "%DeployPath%\" >nul
    echo       [OK] libpq.dll from Qt
    set "LibPqFound=1"
)

REM Check PostgreSQL installations (both standard and custom paths)
if "%LibPqFound%"=="0" (
    for /d %%P in ("C:\Program Files\PostgreSQL\*" "C:\.dev\PostgreSQL\*") do (
        if exist "%%P\bin\libpq.dll" (
            copy /Y "%%P\bin\libpq.dll" "%DeployPath%\" >nul
            echo       [OK] libpq.dll from %%P
            
            REM Copy additional dependencies
            for %%F in ("%%P\bin\libintl-*.dll" "%%P\bin\libiconv-*.dll" "%%P\bin\libcrypto-*.dll" "%%P\bin\libssl-*.dll" "%%P\bin\libwinpthread-*.dll") do (
                if exist "%%F" (
                    copy /Y "%%F" "%DeployPath%\" >nul 2>&1
                    for %%N in ("%%F") do echo       [OK] %%~nxN
                )
            )
            set "LibPqFound=1"
            goto :libpq_found
        )
    )
)
:libpq_found

if "%LibPqFound%"=="0" (
    echo.
    echo       [WARNING] libpq.dll not found!
    echo       To work with PostgreSQL, you need to install PostgreSQL client
    echo       or manually copy libpq.dll to %DeployPath%
    echo.
)

echo.
echo ========================================
echo Deployment Complete!
echo Path: %DeployPath%
echo ========================================
echo.
echo To run: %DeployPath%\%ExeName%
echo.

endlocal


