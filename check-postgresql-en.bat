@echo off
REM Script to check PostgreSQL status
setlocal enabledelayedexpansion

echo ========================================
echo PostgreSQL Status Check
echo ========================================
echo.

REM Check 1: PostgreSQL Service
echo [1/4] Checking PostgreSQL service...
sc query "postgresql*" | find "RUNNING" >nul
if %errorlevel% equ 0 (
    echo       [OK] PostgreSQL service is running
) else (
    echo       [ERROR] PostgreSQL service is NOT running!
    echo.
    echo       Solution:
    echo       1. Open Services.msc (Win+R, type services.msc^)
    echo       2. Find PostgreSQL service
    echo       3. Click "Start"
    echo.
    echo       Or execute in PowerShell as administrator:
    echo       Start-Service postgresql-x64-XX  (replace XX with your version^)
    echo.
)

REM Check 2: Port 5432
echo [2/4] Checking port 5432...
netstat -an | find ":5432" | find "LISTENING" >nul
if %errorlevel% equ 0 (
    echo       [OK] PostgreSQL is listening on port 5432
) else (
    echo       [WARNING] Port 5432 is not listening
)

REM Check 3: psql availability
echo [3/4] Checking psql utility...
where psql >nul 2>&1
if %errorlevel% equ 0 (
    echo       [OK] psql found in PATH
) else (
    echo       [WARNING] psql not found in PATH
    echo       Add PostgreSQL bin path to PATH:
    echo       C:\Program Files\PostgreSQL\XX\bin
    echo       or C:\.dev\PostgreSQL\XX\bin
)

REM Check 4: Database connection attempt
echo [4/4] Attempting database connection...
REM Try to find psql in different versions and locations
set "PSQL_FOUND="
for %%V in (18 17 16 15 14) do (
    if exist "C:\.dev\PostgreSQL\%%V\bin\psql.exe" (
        set "PSQL_FOUND=C:\.dev\PostgreSQL\%%V\bin\psql.exe"
        goto :psql_found
    )
    if exist "C:\Program Files\PostgreSQL\%%V\bin\psql.exe" (
        set "PSQL_FOUND=C:\Program Files\PostgreSQL\%%V\bin\psql.exe"
        goto :psql_found
    )
)
:psql_found
if defined PSQL_FOUND (
    "%PSQL_FOUND%" -h localhost -p 5432 -U postgres -d car_dealership -c "\dt" 2>nul
    if %errorlevel% equ 0 (
        echo       [OK] Connection to car_dealership successful
    ) else (
        echo       [WARNING] Failed to connect to car_dealership
        echo       Database may not exist or password is incorrect
    )
) else (
    echo       [SKIPPED] psql not found, check not performed
)

echo.
echo ========================================
echo Check Complete
echo ========================================
echo.

REM Additional information
echo Connection parameters (default):
echo   Host: localhost
echo   Port: 5432
echo   DB:   car_dealership
echo   User: postgres
echo.

echo To create the database, execute:
echo   psql -U postgres -c "CREATE DATABASE car_dealership;"
echo   psql -U postgres -d car_dealership -f schemas/car-dealership.sql
echo.

pause



