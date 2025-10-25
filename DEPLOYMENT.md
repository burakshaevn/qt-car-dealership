# Развертывание приложения Qt Car Dealership

## Проблема
После использования `windeployqt6` приложение не подключается к базе данных PostgreSQL, потому что драйвер SQL (`qsqlpsql.dll`) не копируется автоматически.

## Решение

### Автоматическое развертывание (Рекомендуется)

1. **Убедитесь, что проект собран:**
   ```powershell
   cd build\Desktop_Qt_6_10_0_MSVC2022_64bit-Debug
   cmake --build . --target qt-car-dealership
   ```

2. **Запустите скрипт развертывания:**
   ```powershell
   .\deploy.ps1
   ```
   
   Или укажите свою папку для развертывания:
   ```powershell
   .\deploy.ps1 -DeployPath "C:\MyApp"
   ```

3. **Готово!** Приложение будет развернуто в папке `.\deploy` со всеми необходимыми библиотеками.

### Ручное развертывание

Если скрипт не работает, выполните вручную:

1. **Создайте папку для развертывания:**
   ```powershell
   mkdir deploy
   ```

2. **Скопируйте .exe файл:**
   ```powershell
   copy build\Desktop_Qt_6_10_0_MSVC2022_64bit-Debug\qt-car-dealership.exe deploy\
   ```

3. **Запустите windeployqt:**
   ```powershell
   C:\.dev\Qt\6.10.0\msvc2022_64\bin\windeployqt6.exe deploy\qt-car-dealership.exe
   ```

4. **Скопируйте драйвер PostgreSQL:**
   ```powershell
   mkdir deploy\sqldrivers
   copy C:\.dev\Qt\6.10.0\msvc2022_64\plugins\sqldrivers\qsqlpsql.dll deploy\sqldrivers\
   ```

5. **Скопируйте libpq.dll** (из установки PostgreSQL):
   ```powershell
   # Найдите libpq.dll в вашей установке PostgreSQL, например:
   copy "C:\Program Files\PostgreSQL\16\bin\libpq.dll" deploy\
   
   # Также может потребоваться скопировать дополнительные DLL:
   copy "C:\Program Files\PostgreSQL\16\bin\libintl-*.dll" deploy\
   copy "C:\Program Files\PostgreSQL\16\bin\libiconv-*.dll" deploy\
   copy "C:\Program Files\PostgreSQL\16\bin\libcrypto-*.dll" deploy\
   copy "C:\Program Files\PostgreSQL\16\bin\libssl-*.dll" deploy\
   ```

6. **Скопируйте ресурсы:**
   ```powershell
   xcopy /E /I resources deploy\resources
   ```

## Проверка драйверов

Чтобы проверить, какие драйверы SQL доступны, можно добавить в код:

```cpp
#include <QSqlDatabase>
qDebug() << "Доступные драйверы SQL:" << QSqlDatabase::drivers();
```

Должен быть в списке: `QPSQL`

## Требования

- Qt 6.10.0 с MSVC 2022 64-bit
- PostgreSQL client библиотеки (libpq.dll)
- Windows 10/11

## Структура развернутого приложения

```
deploy/
├── qt-car-dealership.exe       # Исполняемый файл
├── sqldrivers/                 # SQL драйверы
│   └── qsqlpsql.dll           # Драйвер PostgreSQL
├── resources/                  # Ресурсы приложения
│   ├── *.svg
│   └── */
├── libpq.dll                  # PostgreSQL client library
├── Qt6Core.dll                # Qt библиотеки
├── Qt6Widgets.dll
├── Qt6Gui.dll
└── ... (другие Qt DLL)
```

## Устранение проблем

### "Не удается найти qsqlpsql.dll"
- Убедитесь, что файл находится в папке `deploy\sqldrivers\`
- Проверьте, что путь правильный (не `deploy\plugins\sqldrivers\`)

### "QSqlDatabase: QPSQL driver not loaded"
- Скопируйте `qsqlpsql.dll` в `deploy\sqldrivers\`
- Убедитесь, что `libpq.dll` находится в `deploy\` или в PATH

### "Не удается найти libpq.dll"
- Установите PostgreSQL или скачайте client библиотеки
- Скопируйте `libpq.dll` и зависимости из `C:\Program Files\PostgreSQL\XX\bin\`

### Приложение не запускается
- Запустите из командной строки, чтобы увидеть ошибки:
  ```cmd
  cd deploy
  qt-car-dealership.exe
  ```

