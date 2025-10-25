# Скрипт для развертывания Qt приложения с драйверами SQL
# Использование: .\deploy.ps1 [путь_к_папке_развертывания]

param(
    [string]$DeployPath = ".\deploy"
)

# Пути
$QtPath = "C:\.dev\Qt\6.10.0\msvc2022_64"
$BuildPath = ".\build\Desktop_Qt_6_10_0_MSVC2022_64bit-Debug"
$ExeName = "qt-car-dealership.exe"
$ExePath = Join-Path $BuildPath $ExeName

# Проверка существования исполняемого файла
if (-not (Test-Path $ExePath)) {
    Write-Error "Не найден исполняемый файл: $ExePath"
    Write-Host "Убедитесь, что проект собран."
    exit 1
}

# Создание папки для развертывания
if (-not (Test-Path $DeployPath)) {
    New-Item -ItemType Directory -Path $DeployPath | Out-Null
    Write-Host "Создана папка для развертывания: $DeployPath"
}

# Копирование исполняемого файла
Write-Host "Копирование $ExeName..."
Copy-Item $ExePath -Destination $DeployPath -Force

# Копирование ресурсов
Write-Host "Копирование ресурсов..."
if (Test-Path ".\resources") {
    Copy-Item ".\resources" -Destination $DeployPath -Recurse -Force
}

# Запуск windeployqt
Write-Host "Запуск windeployqt6..."
$WinDeployQt = Join-Path $QtPath "bin\windeployqt6.exe"
$DeployExePath = Join-Path $DeployPath $ExeName

& $WinDeployQt $DeployExePath --no-translations

# Копирование SQL драйверов
Write-Host "Копирование SQL драйверов PostgreSQL..."
$SqlDriversSource = Join-Path $QtPath "plugins\sqldrivers"
$SqlDriversDest = Join-Path $DeployPath "sqldrivers"

if (-not (Test-Path $SqlDriversDest)) {
    New-Item -ItemType Directory -Path $SqlDriversDest | Out-Null
}

# Копирование драйвера PostgreSQL
$PsqlDriver = Join-Path $SqlDriversSource "qsqlpsql.dll"
if (Test-Path $PsqlDriver) {
    Copy-Item $PsqlDriver -Destination $SqlDriversDest -Force
    Write-Host "✓ Скопирован qsqlpsql.dll"
} else {
    Write-Warning "Не найден драйвер PostgreSQL: $PsqlDriver"
}

# Копирование debug версии драйвера (если существует)
$PsqlDriverD = Join-Path $SqlDriversSource "qsqlpsqld.dll"
if (Test-Path $PsqlDriverD) {
    Copy-Item $PsqlDriverD -Destination $SqlDriversDest -Force
    Write-Host "✓ Скопирован qsqlpsqld.dll (debug)"
}

# Копирование libpq.dll (PostgreSQL client library)
Write-Host "Поиск библиотек PostgreSQL..."
$LibPqLocations = @(
    (Join-Path $QtPath "bin\libpq.dll"),
    "C:\Program Files\PostgreSQL\*\bin\libpq.dll",
    "C:\Program Files (x86)\PostgreSQL\*\bin\libpq.dll"
)

$LibPqFound = $false
foreach ($location in $LibPqLocations) {
    $libpqFiles = Get-ChildItem $location -ErrorAction SilentlyContinue
    if ($libpqFiles) {
        foreach ($file in $libpqFiles) {
            Copy-Item $file.FullName -Destination $DeployPath -Force
            Write-Host "✓ Скопирован libpq.dll из: $($file.Directory.FullName)"
            $LibPqFound = $true
            
            # Копирование дополнительных зависимостей PostgreSQL
            $PgBinPath = $file.Directory.FullName
            $PgLibs = @("libintl-*.dll", "libiconv-*.dll", "libcrypto-*.dll", "libssl-*.dll", "libwinpthread-*.dll")
            
            foreach ($lib in $PgLibs) {
                $libFiles = Get-ChildItem (Join-Path $PgBinPath $lib) -ErrorAction SilentlyContinue
                if ($libFiles) {
                    foreach ($libFile in $libFiles) {
                        Copy-Item $libFile.FullName -Destination $DeployPath -Force
                        Write-Host "  ✓ Скопирован $($libFile.Name)"
                    }
                }
            }
            break
        }
    }
    if ($LibPqFound) { break }
}

if (-not $LibPqFound) {
    Write-Warning @"
    
ВНИМАНИЕ: Не найдена библиотека libpq.dll!
Для работы с PostgreSQL необходимо установить PostgreSQL client или скопировать 
libpq.dll вручную в папку $DeployPath

Скачать можно отсюда: https://www.enterprisedb.com/downloads/postgres-postgresql-downloads
"@
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Развертывание завершено!" -ForegroundColor Green
Write-Host "Путь: $DeployPath" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Для запуска выполните: $DeployExePath"

