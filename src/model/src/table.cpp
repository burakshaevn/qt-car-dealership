#include "../include/table.h"
#include "edit_dialog.h"

Table::Table(QSharedPointer<DatabaseHandler> db_manager, const User* user, QWidget* parent)
    : QWidget(parent)
    , m_database_handler(std::move(db_manager))
    , m_data_table(new QTableView(this))
    , m_description_table(new QLabel(this))
{
    // Настройка отображения таблицы
    m_data_table->verticalHeader()->hide();      // Скрываем номера строк
    m_data_table->setAlternatingRowColors(true); // Чередующиеся цвета строк
    m_data_table->setSortingEnabled(true);       // Включаем сортировку
    
    // Настройка заголовков таблицы
    QHeaderView* header = m_data_table->horizontalHeader();
    header->setSortIndicatorShown(true);         // Показываем индикатор сортировки
    header->setSectionsClickable(true);          // Разрешаем клик по заголовкам
}

void Table::BuildAdminTables(){
    m_table_selector = new QComboBox(this);
    m_current_table = Tables::unknown;

    auto* layout = new QVBoxLayout(this);
    m_table_selector->addItems(m_database_handler->GetTables());
    m_table_selector->setCurrentIndex(-1);
    m_table_selector->setStyleSheet(R"(
        QComboBox{
            background-color: #fafafa;
            border: 0px;
            color: #1d1b20;
            padding-left: 27px;
        }

        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border: 1px solid #cccccc;
            background-color: #e0e0e0;
        }
    )");
    layout->addWidget(m_table_selector);
    m_description_table->setStyleSheet(R"(QLabel{\n	color: #1d1b20; \n	font: 18pt "Open Sans"; \n})");
    layout->addWidget(m_description_table);

    // Таблица для отображения данных
    m_data_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_data_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_data_table->horizontalHeader()->setStretchLastSection(true);
    m_data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_data_table->setStyleSheet(R"(
        QTableView{
            border: 0px;
        }
    )");
    layout->addWidget(m_data_table);

    m_floating_menu.reset(new QWidget(this));
    m_floating_menu->setStyleSheet("background-color: #fafafa; border-radius: 29px;");
    m_floating_menu->setFixedSize(335, 74);

    // Горизонтальное размещение кнопок в меню
    QHBoxLayout* menuLayout = new QHBoxLayout(m_floating_menu.get());
    menuLayout->setContentsMargins(20, 10, 20, 10);
    menuLayout->setSpacing(20);

    QPushButton* add_button;     // Кнопка добавления записи
    QPushButton* delete_button;  // Кнопка удаления записи
    QPushButton* edit_button;    // Кнопка редактирования записи
    QPushButton* logout_button;  // Кнопка выхода из системы
    QPushButton* approve_button; // Кнопка подтверждения заявки
    QPushButton* reject_button;  // Кнопка отклонения заявки

    add_button = new QPushButton(QIcon(":/add.svg"), "", m_floating_menu.get());
    add_button->setIconSize(QSize(35, 35));
    add_button->setStyleSheet("QPushButton { border: none; outline: none; }");

    edit_button = new QPushButton(QIcon(":/edit.svg"), "", m_floating_menu.get());
    edit_button->setIconSize(QSize(35, 35));
    edit_button->setStyleSheet("QPushButton { border: none; outline: none; }");

    delete_button = new QPushButton(QIcon(":/delete.svg"), "", m_floating_menu.get());
    delete_button->setIconSize(QSize(35, 35));
    delete_button->setStyleSheet("QPushButton { border: none; outline: none; }");

    logout_button = new QPushButton(QIcon(":/navigate_next.svg"), "", m_floating_menu.get());
    logout_button->setIconSize(QSize(35, 35));
    logout_button->setStyleSheet("QPushButton { border: none; outline: none; }");

    // Создаем кнопки для обработки заявок
    approve_button = new QPushButton(QIcon(":/check.svg"), "", m_floating_menu.get());
    approve_button->setIconSize(QSize(35, 35));
    approve_button->setStyleSheet("QPushButton { border: none; outline: none; }");
    approve_button->setToolTip("Подтвердить заявку");

    reject_button = new QPushButton(QIcon(":/close.svg"), "", m_floating_menu.get());
    reject_button->setIconSize(QSize(35, 35));
    reject_button->setStyleSheet("QPushButton { border: none; outline: none; }");
    reject_button->setToolTip("Отклонить заявку");

    menuLayout->addWidget(approve_button);
    menuLayout->addWidget(reject_button);
    menuLayout->addWidget(add_button);
    menuLayout->addWidget(edit_button);
    menuLayout->addWidget(delete_button);
    menuLayout->addWidget(logout_button);

    // Скрываем кнопки обработки заявок по умолчанию
    ShowRequestButtons(false);

    // Установим позицию меню (по центру внизу)
    m_floating_menu->move(378, 460);
    m_floating_menu->show();

    connect(m_table_selector, &QComboBox::currentTextChanged, this, &Table::LoadTable);
    connect(add_button, &QPushButton::clicked, this, &Table::AddRecord);
    connect(edit_button, &QPushButton::clicked, this, &Table::EditRecord);
    connect(delete_button, &QPushButton::clicked, this, &Table::DeleteRecord);
    connect(logout_button, &QPushButton::clicked, this, &Table::Logout);
    connect(approve_button, &QPushButton::clicked, this, &Table::ApproveRequest);
    connect(reject_button, &QPushButton::clicked, this, &Table::RejectRequest);

    m_floating_menu->installEventFilter(this);
}

bool Table::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_floating_menu.get()) {
        static QPoint dragStartPos;  // Начальная позиция
        static bool dragging = false;

        // Обработка событий мыши
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragStartPos = mouseEvent->pos();
                dragging = true;
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (dragging) {
                QPoint newPos = m_floating_menu->pos() + mouseEvent->pos() - dragStartPos;

                // Убедитесь, что новое положение находится в пределах родительского окна
                QWidget* parent = m_floating_menu->parentWidget();
                if (parent) {
                    QRect parentRect = parent->rect();
                    QSize menuSize = m_floating_menu->size();

                    newPos.setX(std::max(0, std::min(newPos.x(), parentRect.width() - menuSize.width())));
                    newPos.setY(std::max(0, std::min(newPos.y(), parentRect.height() - menuSize.height())));
                }

                m_floating_menu->move(newPos);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            dragging = false; // Завершаем перетаскивание
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void Table::LoadTable() {
    QString table_name = m_table_selector->currentText();

    // Показываем или скрываем кнопки обработки заявок в зависимости от выбранной таблицы
    ShowRequestButtons(IsRequestTable(table_name));

    QString query_str;
    if (table_name == "service_requests") {
        query_str = "SELECT sr.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
                    "c.phone as \"Телефон\", "
                    "cars.name as \"Автомобиль\", "
                    "sr.service_type as \"Тип услуги\", sr.status as \"Статус\", "
                    "sr.created_at as \"Дата создания\", sr.scheduled_date as \"Запланированная дата\" "
                    "FROM service_requests sr "
                    "LEFT JOIN clients c ON sr.client_id = c.id "
                    "LEFT JOIN cars ON sr.car_id = cars.id";
    }
    else if (table_name == "test_drives") {
        query_str = "SELECT td.id as \"№\", "
                   "CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
                   "c.phone as \"Телефон\", "
                   "CONCAT(cars.name, ' (', cars.color, ')') as \"Автомобиль\", "
                   "td.scheduled_date as \"Дата тест-драйва\", "
                   "td.status as \"Статус\", "
                   "td.created_at as \"Дата создания\" "
                   "FROM test_drives td "
                   "LEFT JOIN clients c ON td.client_id = c.id "
                   "LEFT JOIN cars ON td.car_id = cars.id";
    }
    else if (table_name == "insurance_requests") {
        query_str = "SELECT ir.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
                    "c.phone as \"Телефон\", "
                    "cars.name as \"Автомобиль\", "
                    "ir.insurance_type as \"Тип страховки\", ir.status as \"Статус\", "
                    "ir.created_at as \"Дата создания\" "
                    "FROM insurance_requests ir "
                    "LEFT JOIN clients c ON ir.client_id = c.id "
                    "LEFT JOIN cars ON ir.car_id = cars.id";
    }
    else if (table_name == "loan_requests") {
        query_str = "SELECT lr.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
                    "c.phone as \"Телефон\", "
                    "cars.name as \"Автомобиль\", cars.price as \"Цена автомобиля\", "
                    "lr.loan_amount as \"Сумма кредита\", lr.loan_term_months as \"Срок (месяцев)\", "
                    "lr.status as \"Статус\", lr.created_at as \"Дата создания\" "
                    "FROM loan_requests lr "
                    "LEFT JOIN clients c ON lr.client_id = c.id "
                    "LEFT JOIN cars ON lr.car_id = cars.id";
    }
    else if (table_name == "sell_requests") {
        query_str = "SELECT sr.id as \"№\", CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
                    "c.phone as \"Телефон\", "
                    "cars.name as \"Автомобиль\", cars.price as \"Цена\", cars.color as \"Цвет\", "
                    "sr.status as \"Статус\", sr.created_at as \"Дата создания\" "
                    "FROM sell_requests sr "
                    "LEFT JOIN clients c ON sr.client_id = c.id "
                    "LEFT JOIN cars ON sr.car_id = cars.id";
    }
    else if (table_name == "rental_requests") {
        query_str = "SELECT rr.id as \"№\", "
                    "CONCAT(c.first_name, ' ', c.last_name) as \"Клиент\", "
                    "c.phone as \"Телефон\", "
                    "CONCAT(cars.name, ' (', cars.color, ')') as \"Автомобиль\", "
                    "rr.rental_days as \"Дней аренды\", "
                    "rr.start_date as \"Дата начала\", "
                    "rr.status as \"Статус\", "
                    "rr.created_at as \"Дата создания\" "
                    "FROM rental_requests rr "
                    "LEFT JOIN clients c ON rr.client_id = c.id "
                    "LEFT JOIN cars ON rr.car_id = cars.id";
    }
    else {
        query_str = QString("SELECT * FROM %1").arg(table_name);
    }

    qDebug() << "Executing query:" << query_str;

    QVariant result = m_database_handler->ExecuteSelectQuery(query_str);
    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();

        if (!query.isActive()) {
            QMessageBox::critical(this, "Error", "Query execution failed: " + query.lastError().text());
            return;
        }

        if (!query.next()) {
            QMessageBox::warning(this, "Предупреждение", "Выбранная таблица пуста или не существует.");
            // return;
        }

        // Возврат к началу результата
        query.first();

        // Обновление описания таблицы
        m_description_table->clear();
        m_description_table->setText(m_database_handler->GetTableDescription(table_name));
        m_description_table->setWordWrap(true);
        m_description_table->setTextInteractionFlags(Qt::TextBrowserInteraction);
        m_description_table->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        // Создание новой модели
        auto* model = new QStandardItemModel(this);
        m_data_table->setModel(nullptr); // Удаляем старую модель

        // Получение структуры таблицы
        QSqlRecord record = query.record();
        int column_count = record.count();

        // Установка заголовков
        QStringList headers;
        for (int i = 0; i < column_count; ++i) {
            headers << record.fieldName(i); // Имена столбцов уже на русском из SQL запроса
        }
        model->setHorizontalHeaderLabels(headers);

        // Заполнение модели данными из запроса
        int row = 0;
        do {
            QList<QStandardItem*> items;
            for (int col = 0; col < column_count; ++col) {
                QStandardItem* item;
                // Для столбца ID (первый столбец) создаем элемент с числовым значением
                if (col == 0 && (headers[0] == "№" || headers[0] == "id")) {
                    item = new QStandardItem;
                    item->setData(query.value(col).toInt(), Qt::DisplayRole);
                } else {
                    item = new QStandardItem(query.value(col).toString());
                }
                item->setEditable(false);
                items.append(item);
            }
            model->appendRow(items);
            row++;
        } while (query.next());

        // Установка модели
        m_data_table->setModel(model);
        m_data_table->resizeColumnsToContents();
        m_data_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
        m_data_table->setSortingEnabled(true);

        QHeaderView* header = m_data_table->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);

        // Сохранение текущей таблицы
        m_current_table = StringToTables(table_name);
    } else {
        QMessageBox::critical(this, "Ошибка в базе данных", "Ошибка при выполнении запроса.");
    }
}

void Table::AddRecord() {
    if (!m_data_table->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем имя таблицы
    QString tableName = m_table_selector->currentText();
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем модель и создаём пустую запись
    QAbstractItemModel* model = m_data_table->model();
    QSqlRecord newRecord;

    for (int col = 0; col < model->columnCount(); ++col) {
        QString fieldName = model->headerData(col, Qt::Horizontal).toString();
        QSqlField field(fieldName, QVariant::String);

        if (fieldName == "id") {
            // Автоматически вычисляем id
            int newId = m_database_handler->GetMaxOrMinValueFromTable("MAX", fieldName, tableName) + 1;
            field.setValue(newId);
        } else {
            // Для остальных полей задаём пустое значение
            field.setValue("");
        }

        newRecord.append(field);
    }

    try {
        // Открываем диалог EditDialog для ввода данных
        EditDialog dialog(newRecord, this);
        if (dialog.exec() == QDialog::Accepted) {
            QSqlRecord updatedRecord;
            updatedRecord = dialog.GetUpdatedRecord();

            // Формируем SQL-запрос для вставки данных
            QStringList fieldNames, fieldValues;
            for (int col = 0; col < updatedRecord.count(); ++col) {
                QString fieldName = updatedRecord.fieldName(col);
                QString fieldValue = updatedRecord.value(col).toString();

                fieldNames.append(fieldName);
                fieldValues.append("'" + fieldValue + "'");
            }

            QString insertQuery = QString("INSERT INTO %1 (%2) VALUES (%3)")
                                      .arg(tableName)
                                      .arg(fieldNames.join(", "))
                                      .arg(fieldValues.join(", "));

            // Выполняем запрос
            QSqlQuery query;
            if (!query.exec(insertQuery)) {
                throw std::runtime_error(query.lastError().text().toStdString());
            }

            // Перезагружаем таблицу
            LoadTable();
            QMessageBox::information(this, "Информация", "Новая запись добавлена .");
        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void Table::DeleteRecord() {
    if (!m_data_table->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для удаления записи.");
        return;
    }
    QString table_name = m_table_selector->currentText();
    QString col_name = "id";

    int id = -1; // Для хранения идентификатора, если есть primary_key

    if (!col_name.isEmpty()) {
        // Если есть primary_key, запрашиваем ID для удаления
        int min_id = m_database_handler->GetMaxOrMinValueFromTable("MIN", col_name, table_name);
        int max_id = m_database_handler->GetMaxOrMinValueFromTable("MAX", col_name, table_name);

        bool ok;
        id = QInputDialog::getInt(
            this, tr("Удаление записи"), tr("Укажите порядковый номер записи (столбец ID):"), 1, min_id, max_id, 1, &ok
            );

        if (!ok) {
            return;
        }

        if (id < min_id || id > max_id) {
            QMessageBox::warning(this, "Invalid Data", "The entered data number is invalid.");
            return;
        }
    }
    else {
        // Если primary_key отсутствует, используем выбор строки
        QItemSelectionModel* selectionModel = m_data_table->selectionModel();
        if (!selectionModel->hasSelection())
        {
            QMessageBox::warning(this, "No Selection", "Please select a row to delete.");
            return;
        }

        QModelIndexList selectedRows = selectionModel->selectedRows();
        if (selectedRows.size() != 1) {
            QMessageBox::warning(this, "Invalid Selection", "Please select exactly one row to delete.");
            return;
        }

        // Получаем индекс выбранной строки
        QModelIndex selectedIndex = selectedRows.first();
        int row = selectedIndex.row();

        // Получаем данные строки из модели
        QAbstractItemModel* model = m_data_table->model();
        QStringList columnValues;
        for (int col = 0; col < model->columnCount(); ++col) {
            columnValues << model->data(model->index(row, col)).toString();
        }

        // Сформируем запрос для удаления строки на основе значений столбцов
        QString query_string = QString("DELETE FROM %1 WHERE ").arg(table_name);
        QStringList conditions;
        for (int col = 0; col < model->columnCount(); ++col) {
            QString col_name = model->headerData(col, Qt::Horizontal).toString();
            QString value = model->data(model->index(row, col)).toString();
            conditions << QString("%1 = '%2'").arg(col_name, value);
        }
        query_string += conditions.join(" AND ");

        // Подтверждение удаления
        QString warningMessage = "Are you sure you want to delete the selected record?";
        if (QMessageBox::warning(this, "Confirm Deletion", warningMessage, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }

        // Выполняем удаление
        QSqlQuery query;
        if (!query.exec(query_string)) {
            QMessageBox::critical(this, "Error", "Failed to delete record: " + query.lastError().text());
            return;
        }

        LoadTable();
        return;
    }

    // Удаление по primary_key
    if (GetConfirmation(table_name, col_name, id)) {
        QStringList foreignKeys = m_database_handler->GetForeignKeysForColumn(table_name, col_name);

        QString infoMessage;
        if (!foreignKeys.isEmpty()) {
            infoMessage = "Удаление этой записи повлияет на следующие связанные таблицы:\n";
            infoMessage += foreignKeys.join("\n");
            infoMessage += "\n\nВы уверены, что хотите продолжить?";
        }
        else {
            infoMessage = "Вы уверены, что хотите удалить эту запись?";
        }

        if (QMessageBox::warning(this, "Подтверждение удаления", infoMessage, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }

        QSqlQuery query;
        QString query_string = QString("DELETE FROM %1 WHERE %2 = :id").arg(table_name, col_name);

        query.prepare(query_string);
        query.bindValue(":id", id);

        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete record: " + query.lastError().text());
            return;
        }

        LoadTable();
    }
}

void Table::EditRecord() {
    if (!m_data_table->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для редактирования записи.");
        return;
    }

    QString table_name = m_table_selector->currentText();
    QString primary_key_column = GetPrimaryKeyColumnName(table_name);

    if (primary_key_column.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось определить первичный ключ для таблицы.");
        return;
    }

    // Получаем диапазон допустимых ID
    int min_id = m_database_handler->GetMaxOrMinValueFromTable("MIN", primary_key_column, table_name);
    int max_id = m_database_handler->GetMaxOrMinValueFromTable("MAX", primary_key_column, table_name);

    bool ok;
    int id = QInputDialog::getInt(
        this,
        tr("Редактирование записи"),
        tr("Укажите порядковый номер записи (ID):"),
        min_id, min_id, max_id, 1, &ok
        );

    if (!ok) {
        return;
    }

    // Получаем данные записи из БД по ID
    QString query_str = QString("SELECT * FROM %1 WHERE %2 = %3")
                            .arg(table_name)
                            .arg(primary_key_column)
                            .arg(id);

    QSqlQuery query;
    if (!query.exec(query_str) || !query.next()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось найти запись с указанным ID.");
        return;
    }

    // Создаем QSqlRecord с данными найденной записи
    QSqlRecord record = query.record();
    for (int i = 0; i < record.count(); i++) {
        record.setValue(i, query.value(i));
    }

    try {
        // Открываем диалог для редактирования
        EditDialog dialog(record, this);
        if (dialog.exec() == QDialog::Accepted) {
            // Получаем обновлённую запись
            QSqlRecord updatedRecord = dialog.GetUpdatedRecord();

            // Формируем SQL-запрос для обновления строки
            QStringList setClauses;
            for (int col = 0; col < updatedRecord.count(); ++col) {
                QString fieldName = updatedRecord.fieldName(col);
                QString newValue = updatedRecord.value(col).toString();
                setClauses.append(QString("%1 = '%2'").arg(fieldName, newValue));
            }

            QString updateQuery = QString("UPDATE %1 SET %2 WHERE %3 = %4")
                                      .arg(table_name)
                                      .arg(setClauses.join(", "))
                                      .arg(primary_key_column)
                                      .arg(id);

            // Выполняем запрос
            if (!m_database_handler->ExecuteQuery(updateQuery)) {
                throw std::runtime_error("Не удалось выполнить обновление записи.");
            }

            LoadTable();
            QMessageBox::information(this, "Успех", "Запись успешно обновлена.");
        }
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

QString Table::GetPrimaryKeyColumnName(const QString& table_name) {
    QSqlQuery query;
    query.prepare(R"(
        SELECT a.attname
        FROM pg_index i
        JOIN pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey)
        WHERE i.indrelid = :table_name::regclass AND i.indisprimary;
    )");
    query.bindValue(":table_name", table_name);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

bool Table::GetConfirmation(const QString& table_name, const QString& primary_key_column, int id) {
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM %1 WHERE %2 = :id").arg(table_name, primary_key_column));
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(nullptr, "Error", "Failed to fetch record for confirmation: " + query.lastError().text());
        return false;
    }

    // Создаем модальное окно
    QDialog dialog;
    dialog.setWindowTitle("Подтвердите удаление");
    dialog.setModal(true);

    // Создаем табличный виджет
    QTableWidget* tableWidget = new QTableWidget(&dialog);

    QSqlRecord record = query.record();
    int columnCount = record.count();

    // Устанавливаем количество строк и столбцов
    tableWidget->setColumnCount(columnCount);
    tableWidget->setRowCount(1);

    // Устанавливаем заголовки столбцов
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers << record.fieldName(i);
    }
    tableWidget->setHorizontalHeaderLabels(headers);

    // Заполняем данные строки
    for (int i = 0; i < columnCount; ++i) {
        tableWidget->setItem(0, i, new QTableWidgetItem(query.value(i).toString()));
    }

    // Отключаем редактирование
    tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    tableWidget->setSelectionMode(QTableWidget::NoSelection);
    tableWidget->horizontalHeader()->setStretchLastSection(true);

    // Настраиваем размеры таблицы
    tableWidget->resizeColumnsToContents();
    tableWidget->resizeRowsToContents();

    // Установим политику размеров так, чтобы окно занимало минимально необходимый размер
    tableWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Добавляем кнопку подтверждения
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(tableWidget);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* yesButton = new QPushButton("Yes", &dialog);
    QPushButton* noButton = new QPushButton("No", &dialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(yesButton);
    buttonLayout->addWidget(noButton);
    layout->addLayout(buttonLayout);

    // Подгоняем размер окна под содержимое
    dialog.adjustSize();

    // Сигналы кнопок
    QObject::connect(yesButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(noButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // Отображаем окно
    int result = dialog.exec();
    return (result == QDialog::Accepted);
}

bool Table::IsRequestTable(const QString& table_name) const {
    return table_name == "service_requests" ||
           table_name == "insurance_requests" ||
           table_name == "loan_requests" ||
           table_name == "sell_requests" ||
           table_name == "test_drives" ||
           table_name == "rental_requests";
}

void Table::ShowRequestButtons(bool show) {
    if (auto approve = m_floating_menu->findChild<QPushButton*>("approve_button"))
        approve->setVisible(show);
    if (auto reject = m_floating_menu->findChild<QPushButton*>("reject_button"))
        reject->setVisible(show);
}

void Table::UpdateRequestStatus(const QString& table_name, const QString& status, const int request_id) {
    QString query = QString("UPDATE %1 SET status = '%2', notification_shown = false WHERE id = %3")
        .arg(table_name)
        .arg(status)
        .arg(request_id);

    qDebug() << "Executing update query:" << query;

    QVariant result = m_database_handler->ExecuteQuery(query);
    if (!result.toBool()) {
        // QString error = m_database_handler->GetLastError();
        // QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус заявки: " + error);
        // qDebug() << "Update failed:" << error;
        return;
    }

    LoadTable(); // Перезагружаем таблицу для отображения изменений
}

void Table::ApproveRequest()
{
    QModelIndex currentIndex = m_data_table->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::warning(this, "Предупреждение", "Выберите заявку для подтверждения.");
        return;
    }

    int row = currentIndex.row();
    int id = m_data_table->model()->data(m_data_table->model()->index(row, 0)).toInt();
    QString table_name = m_table_selector->currentText();

    // Используем правильные статусы в зависимости от типа таблицы
    QString status;
    if (table_name == "service_requests") {
        status = "подтверждено";
    }
    else {
        status = "одобрено"; // Для loan_requests, insurance_requests и других
    }

    UpdateRequestStatus(table_name, status, id);
}

void Table::RejectRequest()
{
    QModelIndex currentIndex = m_data_table->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::warning(this, "Предупреждение", "Выберите заявку для отклонения.");
        return;
    }

    int row = currentIndex.row();
    int id = m_data_table->model()->data(m_data_table->model()->index(row, 0)).toInt();
    QString table_name = m_table_selector->currentText();

    // Используем правильные статусы в зависимости от типа таблицы
    QString status;
    if (table_name == "service_requests") {
        status = "отменено";
    }
    else {
        status = "отклонено"; // Для loan_requests, insurance_requests и других
    }

    UpdateRequestStatus(table_name, status, id);
}
