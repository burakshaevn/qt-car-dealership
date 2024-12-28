# Автосалон Mercedez-Benz

* C++17, Qt 6.6.3, CMake, PostgreSQL

## Описание
Разработано с помощью C++17 с использованием фреймворка Qt 6.6.3 и системой управления базами данных PostgreSQL. Проект представляет собой десктоп приложение для управления данными автосалона, включая: клиентов, администраторов, автомобили, типы доступных автомобилей и управление покупками. Данные извлекаются из базы данных, обрабатываются и отображаются в интерфейсе с использованием Qt. У администратора есть возможность добавлять, редактировать и удалять данные, у клиента только смотреть/совершать покупки.

В программе предусмотрено два пользователя — **клиент** и **администратор**.

## Сборка и установка
Требования:
- C++17
- Qt 6.6.2+
- PostgreSQL 17+
- CMake 3.5+

Сборка и запуск:

В переменных средах системы должен (PATH) быть указан путь до бинарных библиотек Postgres.
* Windows (_пример_): `C:\dev\PostgreSQL\17\lib`
* MacOS (_пример_): `/Library/PostgreSQL/17/lib`

### Создание базы данных
* В репозитории хранится файл c расширением `.sql`, содержащий пример базы данных для задачи.

  ```
  1. Выполняем авторизацию в pgAdmin 4.
  2. Создаём базу данных, в которой будут расположены таблицы, триггеры и тд.
  3. Открываем «запросник» Query Tool → Вставить код из файла .sql → Выполнить запрос.
  ```

### Сборка под редакторы

* Qt Creator
  ```
  1. Распаковать репозиторий.
  2. Open Qt Creator → Open Project.
  3. В открывшемся окне указываем путь к распакованному репозиторию и выбираем файл CMakeLists.txt.
  4. Выполняем конфигурацию проекта под нужный компилятор «Configure Project».
  ```

* Visual Studio 2022
  ```sh
  cd ..\qt-transport-catalogue\
  mkdir build-vs
  cd ..\qt-transport-catalogue\build-vs
  cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:\Qt\6.6.2\msvc2019_64\"
  mingw32-make
  ```
  где `-DCMAKE_PREFIX_PATH="C:\Qt\6.6.2\msvc2019_64\"` — путь к месту установки фреймворка Qt, собранного с использованием компилятора Microsoft Visual Studio. Здесь можно указать свой компилятор.

## Пример  
<div align="center">
  <img src="https://github.com/user-attachments/assets/175bcaa8-7b76-4191-9ea4-157d21bf97ec" alt="image" width = "70%">
  <p>Рис. 1 — Окно авторизации.</p>
</div> 

<div align="center">
  <img src="https://github.com/user-attachments/assets/c827972d-94c4-4738-bcb3-fa102b1d24ea" alt="image" width = "70%">
  <p>Рис. 2 — Главный экран.</p>
</div> 

## Авторизация
В программе предусмотрено два пользователя — **клиент** и **администратор**.  
<details>
<summary>Авторизация как администратор</summary>
  
  Требуется таблица admins.
  1. Открыть pgAdmin 4
  2. Открыть Query Tool для базы данных
  3. Выполнить SQL-запрос:
  ```sql
  SELECT * FROM admins;
  ```
  4. Скопировать любой email и вставить в поле Login
  5. Скопировать пароль выбранного почтового адреса и вставить в поле Password

</details>

<details>
<summary>Авторизация как клиент</summary>

  Требуется таблица clients.
  1. Открыть pgAdmin 4
  2. Открыть Query Tool для базы данных
  3. Выполнить SQL-запрос:
  ```sql
  SELECT * FROM clients;
  ```
  5. Скопировать любой email и вставить в поле Login
  6. Скопировать пароль выбранного почтового адреса и вставить в поле Password

</details>

## Возможности клиента
После авторизации появляется плавающее меню. Доступно 4 кнопки — _главная, поиск, сортировка по цветам и профиль_. 

* **Главна страница**. Здесь все автомобили по умолчанию отображаются в белом цвете.
  <div align="center">
    <img src="https://github.com/user-attachments/assets/d816cd7c-a3d5-4bf5-95e9-e1c73af36c99" alt="GIF Image" width="50%">
    <p>Рис. 3 — Главная страница после авторизации как пользователь.</p>
  </div>  
  После выбора автомобиля в прокручиваемом меню, осуществляется переход на отдельную страницу, где пользователю предосталяется возможность выбрать цвет автомобиля и при необходимости произвести оплату. 
  <div align="center">
    <img src="https://github.com/user-attachments/assets/5b170d61-ec35-4c35-9aab-a3b0325ddc44" alt="Image" width="50%">
    <p>Рис. 4 — Главная страница после авторизации как пользователь.</p>
  </div> 
  В правом верхем углу карточки автомобиля отображается доступное количество цветов.
  <div align="center">
    <img src="https://github.com/user-attachments/assets/194d2e44-b0bc-457b-a23d-de0d56d26c67" alt="GIF Image" width="50%">
    <p>Рис. 5 — Главная страница после авторизации как пользователь.</p>
  </div> 
* **Сортировка по цветам**.
  <div align="center">
    <img src="https://github.com/user-attachments/assets/012c00f8-50ea-4b43-9111-bc45df9c9bec" alt="GIF Image" width="50%">
    <p>Рис. 6 — Главная страница после авторизации как пользователь.</p>
  </div> 

* **Оплата**. Оплата происходит после нажатия на кнопку «К оплате». 

  Выполняется запрос `INSERT`в базу данных:
  ```sql
  INSERT INTO public.purchases(id, car_id, client_id) VALUES (%1, %2, %3);
  ```
  где `%1` — порядковый номер новой записи в таблице `purchases` (он подбирается автоматически), `%2` — id купленного автомобиля (ссылка на столбец id из таблицы cars), `%3` — id клиента (ссылка на столбец id из таблицы clients).
  
  Купленный автомобиль отображается в таблице `purchases` и отображается в профиле.
  <div align="center">
    <img src="https://github.com/user-attachments/assets/4c5ddf08-83e5-4787-9479-606c51aedeed" alt="Image" width="40%">
    <p>Рис. 8 — Подтверждение операции.</p>
  </div> 
  
* **Профиль**. Здесь отображается прокручиваемый вниз виджет, который содержит в себе карточки купленных автомобилей клиентом. 
  <div align="center">
    <img src="https://github.com/user-attachments/assets/e44b73a2-3d1f-4303-9877-7e4df31be462" alt="Image" width="50%">
    <p>Рис. 9 — Страница профиля.</p>
  </div> 
  
## Возможности администратора
По умолчанию эта страница будет пустой до тех пор, пока не будет выбрана таблица для редактирования. Чтобы это сделать, необходимо нажать на «меню» в верхней части экрана. Это выпадающий список, в который динамически добавятся все таблицы базы данных. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/c27aac2c-f20b-4407-8308-9b1806ae218f" alt="image" width="50%">
  <p>Рис. N — Страница по умолчанию для администратора. (teachers).</p>
</div>

После выбора таблицы её данные отображаются на экране.
<div align="center">
  <img src="https://github.com/user-attachments/assets/07f5a53c-fda0-4061-af55-70510eec565c" alt="image" width="50%">
  <p>Рис. N — Выгрузка таблицы.</p>
</div>

### Добавление записи
При нажатии на кнопку добавления новой записи «Добавить», происходит вызов функции `void Table::AddRecord();`, которая создаёт экземпляр класса, в котором строится диалоговое окно `EditDialog dialog(newRecord, this);`, где `newRecord` — передача конкретной записи из базы данных в виде QSqlRecord. Эта запись используется для отображения столбцов таблицы.

Здесь мы не можем задать значение поля id. Потому что это «счётчик» записей в таблице, который будет сам автоматически увеличиваться по мере поступления новых записей.
<div align="center">
  <img src="https://github.com/user-attachments/assets/a9702f08-d5af-47cc-ac8b-094dbe3f956d" alt="image">
  <p>Рис. 5 — Окно для добавления.</p>
</div>

## Удаление записи
Производится ввод ID записи в таблице. Она будет удалена.

В некоторых таблицах, например, ID начинается необязательно с 1. Итерироваться в этом окне мы можем от самого минимального ID до самого максимального. Чтобы не выходить за пределы.
<div align="center">
  <img src="https://github.com/user-attachments/assets/a3b3e4f2-aa02-4b36-83c0-fb953ccb189e" alt="image">
  <p>Рис. 5 — Окно ввода ID записи (автоинкрементируемого столбца) в текущей таблице.</p>
</div>

После указания ID удаляемой записи выходит окно подтверждения удаления, где строится таблица с удаляемой строкой (чтобы быть уверенным, что удаляется именно то, что мы задумали).
<div align="center">
  <img src="https://github.com/user-attachments/assets/fed447e6-314a-4583-9b9e-654bc2e0908f" alt="image">
  <p>Рис. 6 — Вывод удаляемой строки.</p>
</div>

<div align="center">
  <img src="https://github.com/user-attachments/assets/bf66e036-a05d-48cb-a64a-5b911e22aef6" alt="image">
  <p>Рис. 7 — Окно подтверждения удаления.</p>
</div>

После подтверждения происходит удаление из основной таблицы, **а записи, которые ссылались на первичный ключ этой строки, каскадно удаляются**.

## Редактирование записи
Для редактирования указывается всегда номер строки в таблицы, а не ID записи. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/b44792b7-1329-4303-8a26-cb23624892cf" alt="image">
  <p>Рис. 8 — Окно поиска записи в таблице.</p>
</div>

<div align="center">
  <img src="https://github.com/user-attachments/assets/0e3a0c6f-5d6f-4b74-9475-12cfbcc66ccc" alt="image">
  <p>Рис. 9 — Окно редактирования записи.</p>
</div>

## ER-диаграмма базы данных
<div align="center">
  <img src="https://github.com/user-attachments/assets/ee38b467-5e2b-45b6-bf15-3c12035b01cb" alt="image">
  <p>Рис. 10 — ER-диаграмма.</p>
</div>    
