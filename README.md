# Десктоп приложение «Управление автосалоном Mercedez-Benz» 
Основная задача — демонстрация навыков работы с **С++17**, СУБД **PostgresSQL** и фреймворком **Qt 6.8.1**.

## Описание
Разработано с помощью C++17 с использованием фреймворка Qt 6.6.3 и системой управления базами данных PostgreSQL. Приложение предоставляет средства для управления данными автосалона, включая: клиентов, администраторов, автомобили, подборки автомобилей и управление покупками. Данные извлекаются из базы данных, локально кэшируются и отображаются в интерфейсе с использованием Qt. 

В программе предусмотрено два пользователя — **клиент** и **администратор**.
* Администратор может добавлять, редактировать и удалять данные.
* Клиент может просматривать подборки, совершать покупки и имеет личный кабинет.

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
  <img src="https://github.com/user-attachments/assets/5b8423ba-f1b0-43e3-817d-efa325d51fa1" alt="image">
  <p>Рис. 1 — Главный экран.</p>
</div> 

## Авторизация
Для всех видов пользователя единое окно авторизации.
<div align="center">
  <img src="https://github.com/user-attachments/assets/67a30c84-0cd7-4dee-854d-a5c2414cbfdd" alt="image" width = "70%">
  <p>Рис. 2 — Окно авторизации.</p>
</div> 

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
    <img src="https://github.com/user-attachments/assets/f3cabfc6-5cde-4b71-82a8-ea636b6f801b" alt="GIF Image" width="70%">
    <p>Рис. 3 — Главная страница после авторизации как пользователь.</p>
  </div>  
  После выбора автомобиля в прокручиваемом меню, осуществляется переход на отдельную страницу, где пользователю предосталяется возможность выбрать цвет автомобиля и при необходимости произвести оплату. 
  <div align="center">
    <img src="https://github.com/user-attachments/assets/56100e3c-ff71-4d0f-b846-5a1eab8d5224" alt="Image" width="70%">
    <p>Рис. 4 — Персональная страница автомобиля с возможностью выбора цветов и покупки.</p>
  </div> 
  В правом верхем углу карточки автомобиля отображается доступное количество цветов.
  <div align="center">
    <img src="https://github.com/user-attachments/assets/6ef7c5f6-423b-4f29-8373-f525af132b0d" alt="GIF Image" width="70%">
    <p>Рис. 5 — Персональная страница автомобиля с возможностью выбора цветов и покупки.</p>
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
    <img src="https://github.com/user-attachments/assets/82db5d06-3786-4fcc-8b8f-05572f31abcf" alt="Image">
    <p>Рис. 8 — Подтверждение операции.</p>
  </div> 
  
* **Профиль**. Здесь отображается прокручиваемый вниз виджет, который содержит в себе карточки купленных автомобилей клиентом. 
  <div align="center">
    <img src="https://github.com/user-attachments/assets/811734de-7b6e-4112-a03b-0cd467f47e70" alt="Image" width="70%">
    <p>Рис. 9 — Страница профиля.</p>
  </div> 
  
## Возможности администратора
По умолчанию эта страница будет пустой до тех пор, пока не будет выбрана таблица для редактирования. Чтобы это сделать, необходимо нажать на выпадающий список в верхней части экрана. Это виджет, в который динамически добавятся все таблицы из базы данных. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/738d6875-7a6d-4339-9087-ec31fe93c6a8" alt="image" width="50%">
  <p>Рис. 10 — Страница по умолчанию для администратора.</p>
</div>

После выбора таблицы её данные отображаются на экране.
<div align="center">
  <img src="https://github.com/user-attachments/assets/6e6dde3e-73b5-4ed5-9f40-02736aefaff1" alt="image" width="50%">
  <p>Рис. 11 — Выгрузка данных из таблицы.</p>
</div>

Если плавающее меню администратора перекрывает часть данных, есть возможность его перетаскивать в пределах экрана. Логику реализует сигнал `bool Table::eventFilter(QObject* obj, QEvent* event)`.
<div align="center">
  <img src="https://github.com/user-attachments/assets/4a039503-255c-492b-be0c-be83d37ccd1a" alt="image" width="50%">
  <p>Рис. 12 — Динамическое изменение позиции плавающего меню.</p>
</div>

### Добавление записи
После нажатия на «Добавить», происходит вызов функции `void Table::AddRecord();`, которая в себе создаёт экземпляр класса EditDialog, в котором строится диалоговое окно `EditDialog dialog(newRecord, this);`, где `newRecord` — передача конкретной записи из базы данных в виде QSqlRecord. Эта запись используется для отображения столбцов таблицы.

Здесь нельзя задать значение поля id. Потому что это «счётчик» записей в таблице, который будет сам автоматически увеличиваться по мере поступления новых записей.
<div align="center">
  <img src="https://github.com/user-attachments/assets/ce6f503a-9ce2-475b-a345-abce94b22846" alt="image">
  <p>Рис. 13 — Окно для добавления.</p>
</div>

## Удаление записи
Производится ввод ID записи в таблице. Она будет удалена.

В некоторых таблицах, например, ID начинается необязательно с 1. Итерироваться в этом окне мы можем от самого минимального ID до самого максимального. Чтобы не выходить за пределы.
<div align="center">
  <img src="https://github.com/user-attachments/assets/93fefce8-ee9c-40b0-a120-2a9f731b8ea1" alt="image">
  <p>Рис. 14 — Окно ввода ID записи (автоинкрементируемого столбца) в текущей таблице.</p>
</div>

После указания ID удаляемой записи выходит окно подтверждения удаления, где строится таблица с удаляемой строкой (чтобы быть уверенным, что удаляется именно то, что мы задумали).
<div align="center">
  <img src="https://github.com/user-attachments/assets/39c3bbce-84ff-4ba9-b20f-dcee17e80708" alt="image">
  <p>Рис. 15 — Вывод удаляемой строки.</p>
</div>

После подтверждения происходит удаление из основной таблицы, **а записи, которые ссылались на первичный ключ этой строки, каскадно удаляются**.
<div align="center">
  <img src="https://github.com/user-attachments/assets/afc0ea81-7648-4562-bcf3-43e3f9d9248b" alt="image">
  <p>Рис. 16 — Окно подтверждения удаления.</p>
</div>

## Редактирование записи
Для редактирования указывается всегда номер строки в таблицы, а не ID записи. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/d7c0b9b5-9a02-4260-aff5-85586e22d4ac" alt="image">
  <p>Рис. 17 — Окно поиска записи в таблице.</p>
</div>

Здесь редактирование порядкового номера записи (id) также недоступно.
<div align="center">
  <img src="https://github.com/user-attachments/assets/08848999-eab6-4a4e-bbb5-b9fc026870e1" alt="image">
  <p>Рис. 18 — Окно редактирования записи.</p>
</div>

## ER-диаграмма базы данных
<div align="center">
  <img src="https://github.com/user-attachments/assets/96c5a4b0-6d4f-41f4-a66d-a5c035a8c498" alt="image">
  <p>Рис. 19 — ER-диаграмма.</p>
</div>    
