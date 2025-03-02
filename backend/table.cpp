#include "table.h"

Table::Table(std::shared_ptr<DatabaseHandler> db_manager, const User* user, QWidget* parent)
    : QWidget(parent)
    , db_manager_(std::move(db_manager))
    , data_table_(new QTableView(this))
    , description_table(new QLabel(this))
{}

void Table::BuildAdminTables(){
    table_selector_ = new QComboBox(this);
    current_table_ = Tables::unknown;

    auto* layout = new QVBoxLayout(this);
    table_selector_->addItems(db_manager_->GetTables());
    table_selector_->setCurrentIndex(-1);
    table_selector_->setStyleSheet(R"(
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
    layout->addWidget(table_selector_);
    description_table->setStyleSheet(R"(QLabel{\n	color: #1d1b20; \n	font: 18pt "Open Sans"; \n})");
    layout->addWidget(description_table);

    // Таблица для отображения данных
    data_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    data_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    data_table_->horizontalHeader()->setStretchLastSection(true);
    data_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    data_table_->setStyleSheet(R"(
        QTableView{
            border: 0px;
        }
    )");
    layout->addWidget(data_table_);

    floating_menu_ = std::make_unique<QWidget>(this);
    floating_menu_->setStyleSheet("background-color: #fafafa; border-radius: 29px;");
    floating_menu_->setFixedSize(335, 74);

    // Горизонтальное размещение кнопок в меню
    QHBoxLayout* menuLayout = new QHBoxLayout(floating_menu_.get());
    menuLayout->setContentsMargins(20, 10, 20, 10);
    menuLayout->setSpacing(20);

    add_button_ = new QPushButton(QIcon(":/add.svg"), "", floating_menu_.get());
    add_button_->setIconSize(QSize(35, 35));
    add_button_->setStyleSheet("QPushButton { border: none; outline: none; }");

    edit_button_ = new QPushButton(QIcon(":/edit.svg"), "", floating_menu_.get());
    edit_button_->setIconSize(QSize(35, 35));
    edit_button_->setStyleSheet("QPushButton { border: none; outline: none; }");

    delete_button_ = new QPushButton(QIcon(":/delete.svg"), "", floating_menu_.get());
    delete_button_->setIconSize(QSize(35, 35));
    delete_button_->setStyleSheet("QPushButton { border: none; outline: none; }");

    logout_button_ = new QPushButton(QIcon(":/navigate_next.svg"), "", floating_menu_.get());
    logout_button_->setIconSize(QSize(35, 35));
    logout_button_->setStyleSheet("QPushButton { border: none; outline: none; }");

    menuLayout->addWidget(add_button_);
    menuLayout->addWidget(edit_button_);
    menuLayout->addWidget(delete_button_);
    menuLayout->addWidget(logout_button_);

    // Установим позицию меню (по центру внизу)
    floating_menu_->move(378, 460);
    floating_menu_->show();

    connect(table_selector_, &QComboBox::currentTextChanged, this, &Table::LoadTable);
    connect(add_button_, &QPushButton::clicked, this, &Table::AddRecord);
    connect(edit_button_, &QPushButton::clicked, this, &Table::EditRecord);
    connect(delete_button_, &QPushButton::clicked, this, &Table::DeleteRecord);
    connect(logout_button_, &QPushButton::clicked, this, &Table::Logout);

    floating_menu_->installEventFilter(this);
}

bool Table::eventFilter(QObject* obj, QEvent* event) {
    if (obj == floating_menu_.get()) {
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
                QPoint newPos = floating_menu_->pos() + mouseEvent->pos() - dragStartPos;

                // Убедитесь, что новое положение находится в пределах родительского окна
                QWidget* parent = floating_menu_->parentWidget();
                if (parent) {
                    QRect parentRect = parent->rect();
                    QSize menuSize = floating_menu_->size();

                    newPos.setX(std::max(0, std::min(newPos.x(), parentRect.width() - menuSize.width())));
                    newPos.setY(std::max(0, std::min(newPos.y(), parentRect.height() - menuSize.height())));
                }

                floating_menu_->move(newPos);
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
    QString table_name = table_selector_->currentText();

    QVariant result = db_manager_->ExecuteSelectQuery(QString("SELECT * FROM public.%1").arg(table_name));
    if (result.canConvert<QSqlQuery>()) {
        QSqlQuery query = result.value<QSqlQuery>();

        if (!query.isActive()) {
            QMessageBox::critical(this, "Error", "Query execution failed: " + query.lastError().text());
            return;
        }

        if (!query.next()) {
            QMessageBox::warning(this, "Предупреждение", "Выбранная таблица пуста или не существует.");
            return;
        }

        // Возврат к началу результата
        query.first();

        // Обновление описания таблицы
        description_table->clear();
        description_table->setText(db_manager_->GetTableDescription(table_name));
        description_table->setWordWrap(true);
        description_table->setTextInteractionFlags(Qt::TextBrowserInteraction);
        description_table->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        // Создание новой модели
        auto* model = new QStandardItemModel(this);
        data_table_->setModel(nullptr); // Удаляем старую модель

        // Получение структуры таблицы
        QSqlRecord record = query.record();
        int column_count = record.count();

        // Установка заголовков
        QStringList headers;
        for (int i = 0; i < column_count; ++i) {
            headers << record.fieldName(i); // Имена столбцов
        }
        model->setHorizontalHeaderLabels(headers);

        // Заполнение модели данными из запроса
        do {
            QList<QStandardItem*> items;
            for (int col = 0; col < column_count; ++col) {
                auto* item = new QStandardItem(query.value(col).toString());
                item->setEditable(false);
                items.append(item);
            }
            model->appendRow(items);
        } while (query.next());

        // Установка модели
        data_table_->setModel(model);
        data_table_->resizeColumnsToContents();
        data_table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
        data_table_->setSortingEnabled(true);

        QHeaderView* header = data_table_->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);

        // Сохранение текущей таблицы
        current_table_ = StringToTables(table_name);
    } else {
        QMessageBox::critical(this, "Ошибка в базе данных", "Ошибка при выполнении запроса.");
    }
}

void Table::AddRecord() {
    if (!data_table_->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем имя таблицы
    QString tableName = table_selector_->currentText();
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем модель и создаём пустую запись
    QAbstractItemModel* model = data_table_->model();
    QSqlRecord newRecord;

    for (int col = 0; col < model->columnCount(); ++col) {
        QString fieldName = model->headerData(col, Qt::Horizontal).toString();
        QSqlField field(fieldName, QVariant::String);

        if (fieldName == "id") {
            // Автоматически вычисляем id
            int newId = db_manager_->GetMaxOrMinValueFromTable("MAX", fieldName, tableName) + 1;
            field.setValue(newId);
        } else {
            // Для остальных полей задаём пустое значение
            field.setValue("");
        }

        newRecord.append(field);
    }

    try{
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
    catch (const std::exception& e){
        QMessageBox::critical(this, "Error", e.what());
    }
}

void Table::DeleteRecord() {
    if (!data_table_->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для удаления записи.");
        return;
    }
    QString table_name = table_selector_->currentText();
    QString col_name = "id";

    int id = -1; // Для хранения идентификатора, если есть primary_key

    if (!col_name.isEmpty()) {
        // Если есть primary_key, запрашиваем ID для удаления
        int min_id = db_manager_->GetMaxOrMinValueFromTable("MIN", col_name, table_name);
        int max_id = db_manager_->GetMaxOrMinValueFromTable("MAX", col_name, table_name);

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
    } else {
        // Если primary_key отсутствует, используем выбор строки
        QItemSelectionModel* selectionModel = data_table_->selectionModel();
        if (!selectionModel->hasSelection()) {
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
        QAbstractItemModel* model = data_table_->model();
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
        if (QMessageBox::warning(this, "Confirm Deletion", warningMessage,
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
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
        QStringList foreignKeys = db_manager_->GetForeignKeysForColumn(table_name, col_name);

        QString infoMessage;
        if (!foreignKeys.isEmpty()) {
            infoMessage = "Удаление этой записи повлияет на следующие связанные таблицы:\n";
            infoMessage += foreignKeys.join("\n");
            infoMessage += "\n\nВы уверены, что хотите продолжить?";
        } else {
            infoMessage = "Вы уверены, что хотите удалить эту запись?";
        }

        if (QMessageBox::warning(this, "Подтверждение удаления", infoMessage,
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }

        QSqlQuery query;
        QString query_string = QString("DELETE FROM %1 WHERE %2 = :id")
                                   .arg(table_name, col_name);

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
    if (!data_table_->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для редактирования записи.");
        return;
    }

    bool ok;
    int row = QInputDialog::getInt(
        this, "Редактирование записи", "Укажите порядковый номер строки в таблице:", 1, 1, data_table_->model()->rowCount(), 1, &ok
        );

    if (!ok) {
        return;
    }

    row -= 1;

    if (row < 0 || row >= data_table_->model()->rowCount()) {
        QMessageBox::warning(this, "Invalid Data", "The entered data number is invalid.");
        return;
    }

    // Получаем данные текущей строки через модель
    QAbstractItemModel* model = data_table_->model();
    QSqlRecord record;
    for (int col = 0; col < model->columnCount(); ++col) {
        QVariant value = model->index(row, col).data();
        record.append(QSqlField(model->headerData(col, Qt::Horizontal).toString(), value.type()));
        record.setValue(col, value);
    }

    try{
        // Открываем диалог для редактирования
        EditDialog dialog(record, this);
        if (dialog.exec() == QDialog::Accepted) {
            // Получаем обновлённую запись
            QSqlRecord updatedRecord = dialog.GetUpdatedRecord();

            // Формируем SQL-запрос для обновления строки
            QString tableName = table_selector_->currentText();
            QStringList setClauses;
            QString whereClause;

            // Формирование SET (новые значения)
            for (int col = 0; col < updatedRecord.count(); ++col) {
                QString fieldName = updatedRecord.fieldName(col);
                QString newValue = updatedRecord.value(col).toString();
                setClauses.append(QString("%1 = '%2'").arg(fieldName, newValue));
            }

            // Формирование WHERE (по всем данным строки)
            for (int col = 0; col < record.count(); ++col) {
                QString fieldName = record.fieldName(col);
                QString oldValue = record.value(col).toString();

                if (!whereClause.isEmpty()) {
                    whereClause += " AND ";
                }
                whereClause += QString("%1 = '%2'").arg(fieldName, oldValue);
            }

            if (whereClause.isEmpty()) {
                throw std::runtime_error("Cannot determine the row for update.");
            }

            QString updateQuery = QString("UPDATE %1 SET %2 WHERE %3")
                                      .arg(tableName, setClauses.join(", "), whereClause);

            // Выполняем запрос
            QSqlQuery query;
            if (!query.exec(updateQuery)) {
                throw std::runtime_error(query.lastError().text().toStdString());
            }

            LoadTable();
            QMessageBox::information(this, "Success", "Record updated successfully.");
        }
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
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
