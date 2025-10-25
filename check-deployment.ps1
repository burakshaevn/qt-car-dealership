# Скрипт для проверки правильности развертывания
param(
    [string]$DeployPath = ".\deploy"
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Проверка развертывания Qt Car Dealership" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$issues = @()
$warnings = @()

# Проверка 1: Существует ли папка развертывания
Write-Host "[1/7] Проверка папки развертывания..." -NoNewline
if (Test-Path $DeployPath) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ✗" -ForegroundColor Red
    $issues += "Папка развертывания не найдена: $DeployPath"
}

# Проверка 2: Исполняемый файл
Write-Host "[2/7] Проверка исполняемого файла..." -NoNewline
$exePath = Join-Path $DeployPath "qt-car-dealership.exe"
if (Test-Path $exePath) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ✗" -ForegroundColor Red
    $issues += "Исполняемый файл не найден: $exePath"
}

# Проверка 3: Папка sqldrivers
Write-Host "[3/7] Проверка папки sqldrivers..." -NoNewline
$sqlDriversPath = Join-Path $DeployPath "sqldrivers"
if (Test-Path $sqlDriversPath) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ✗" -ForegroundColor Red
    $issues += "Папка sqldrivers не найдена: $sqlDriversPath"
}

# Проверка 4: Драйвер PostgreSQL
Write-Host "[4/7] Проверка драйвера PostgreSQL..." -NoNewline
$psqlDriver = Join-Path $sqlDriversPath "qsqlpsql.dll"
if (Test-Path $psqlDriver) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ✗" -ForegroundColor Red
    $issues += "Драйвер PostgreSQL не найден: $psqlDriver"
}

# Проверка 5: libpq.dll
Write-Host "[5/7] Проверка libpq.dll..." -NoNewline
$libpqPath = Join-Path $DeployPath "libpq.dll"
if (Test-Path $libpqPath) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ⚠" -ForegroundColor Yellow
    $warnings += "libpq.dll не найден в папке развертывания. Может потребоваться для подключения к PostgreSQL."
}

# Проверка 6: Qt DLL
Write-Host "[6/7] Проверка Qt библиотек..." -NoNewline
$qtDlls = @("Qt6Core.dll", "Qt6Widgets.dll", "Qt6Gui.dll", "Qt6Sql.dll")
$missingQtDlls = @()
foreach ($dll in $qtDlls) {
    $dllPath = Join-Path $DeployPath $dll
    if (-not (Test-Path $dllPath)) {
        $missingQtDlls += $dll
    }
}
if ($missingQtDlls.Count -eq 0) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ✗" -ForegroundColor Red
    $issues += "Отсутствуют Qt DLL: $($missingQtDlls -join ', ')"
}

# Проверка 7: Ресурсы
Write-Host "[7/7] Проверка ресурсов..." -NoNewline
$resourcesPath = Join-Path $DeployPath "resources"
if (Test-Path $resourcesPath) {
    Write-Host " ✓" -ForegroundColor Green
} else {
    Write-Host " ⚠" -ForegroundColor Yellow
    $warnings += "Папка resources не найдена. Приложение может не отображать изображения."
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

# Вывод результатов
if ($issues.Count -eq 0 -and $warnings.Count -eq 0) {
    Write-Host "Все проверки пройдены успешно! ✓" -ForegroundColor Green
} else {
    if ($issues.Count -gt 0) {
        Write-Host ""
        Write-Host "КРИТИЧЕСКИЕ ПРОБЛЕМЫ:" -ForegroundColor Red
        foreach ($issue in $issues) {
            Write-Host "  ✗ $issue" -ForegroundColor Red
        }
    }
    
    if ($warnings.Count -gt 0) {
        Write-Host ""
        Write-Host "ПРЕДУПРЕЖДЕНИЯ:" -ForegroundColor Yellow
        foreach ($warning in $warnings) {
            Write-Host "  ⚠ $warning" -ForegroundColor Yellow
        }
    }
    
    Write-Host ""
    if ($issues.Count -gt 0) {
        Write-Host "Рекомендация: Запустите .\deploy.ps1 для правильного развертывания" -ForegroundColor Cyan
    }
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Дополнительная информация
if (Test-Path $exePath) {
    Write-Host "Размер исполняемого файла: $([math]::Round((Get-Item $exePath).Length / 1MB, 2)) MB"
}

if (Test-Path $DeployPath) {
    $totalSize = (Get-ChildItem $DeployPath -Recurse | Measure-Object -Property Length -Sum).Sum
    Write-Host "Общий размер развертывания: $([math]::Round($totalSize / 1MB, 2)) MB"
    $fileCount = (Get-ChildItem $DeployPath -Recurse -File).Count
    Write-Host "Всего файлов: $fileCount"
}

