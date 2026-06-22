#include "../include/AdminTableWidget.h"
#include "AdminRecordEditDialog.h"
#include "ThemeStyleProvider.h"
#include <QWidget>
#include <QString>

AdminTableWidget::AdminTableWidget(QSharedPointer<DatabaseHandler> db_manager, QWidget* parent)
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

void AdminTableWidget::BuildAdminTables(){
    m_table_selector = new QComboBox(this);
    auto* layout = new QVBoxLayout(this);
    
    QSqlQuery tables_query;
    if (tables_query.exec("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public' ORDER BY table_name")) {
        while (tables_query.next()) {
            const QString table_name = tables_query.value(0).toString();
            m_table_selector->addItem(table_name, table_name);
        }
    }
    
    m_table_selector->setCurrentIndex(-1);
    ApplyThemeStyle(m_table_selector, "AdminTableSelector");
    layout->addWidget(m_table_selector);
    ApplyThemeStyle(m_description_table, "AdminDescription");
    layout->addWidget(m_description_table);

    // Таблица для отображения данных
    m_data_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_data_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_data_table->horizontalHeader()->setStretchLastSection(true);
    m_data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ApplyThemeStyle(m_data_table, "AdminTableView");
    layout->addWidget(m_data_table);

    m_floating_menu.reset(new QWidget(this));
    ApplyThemeStyle(m_floating_menu.get(), "AdminFloatingMenu");
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

    add_button = new QPushButton("", m_floating_menu.get());
    ApplyThemeIcon(add_button, "add.svg");
    add_button->setIconSize(QSize(35, 35));
    ApplyThemeStyle(add_button, "IconButton");

    edit_button = new QPushButton("", m_floating_menu.get());
    ApplyThemeIcon(edit_button, "edit.svg");
    edit_button->setIconSize(QSize(35, 35));
    ApplyThemeStyle(edit_button, "IconButton");

    delete_button = new QPushButton("", m_floating_menu.get());
    ApplyThemeIcon(delete_button, "delete.svg");
    delete_button->setIconSize(QSize(35, 35));
    ApplyThemeStyle(delete_button, "IconButton");

    logout_button = new QPushButton("", m_floating_menu.get());
    ApplyThemeIcon(logout_button, "navigate_next.svg");
    logout_button->setIconSize(QSize(35, 35));
    ApplyThemeStyle(logout_button, "IconButton");

    // Создаем кнопки для обработки заявок
    approve_button = new QPushButton("", m_floating_menu.get());
    ApplyThemeIcon(approve_button, "check.svg");
    approve_button->setIconSize(QSize(35, 35));
    ApplyThemeStyle(approve_button, "IconButton");
    approve_button->setToolTip("Подтвердить заявку");

    reject_button = new QPushButton("", m_floating_menu.get());
    ApplyThemeIcon(reject_button, "close.svg");
    reject_button->setIconSize(QSize(35, 35));
    ApplyThemeStyle(reject_button, "IconButton");
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

    connect(m_table_selector, &QComboBox::currentTextChanged, this, &AdminTableWidget::LoadTable);
    connect(add_button, &QPushButton::clicked, this, &AdminTableWidget::AddRecord);
    connect(edit_button, &QPushButton::clicked, this, &AdminTableWidget::EditRecord);
    connect(delete_button, &QPushButton::clicked, this, &AdminTableWidget::DeleteRecord);
    connect(logout_button, &QPushButton::clicked, this, &AdminTableWidget::Logout);
    connect(approve_button, &QPushButton::clicked, this, &AdminTableWidget::ApproveRequest);
    connect(reject_button, &QPushButton::clicked, this, &AdminTableWidget::RejectRequest);

    m_floating_menu->installEventFilter(this);
}

bool AdminTableWidget::eventFilter(QObject* obj, QEvent* event) {
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

void AdminTableWidget::LoadTable() {
    QString table_name = m_table_selector->currentData().toString();
    if (table_name.isEmpty()) {
        table_name = m_table_selector->currentText();
    }

    ShowRequestButtons(AdminTableModel::IsRequestTableName(table_name));

    if (!m_table_model) {
        m_table_model.reset(new AdminTableModel(this));
    }

    if (!m_table_model->Load(table_name)) {
        QMessageBox::critical(this, "Error", "Query execution failed: " + m_table_model->lastError().text());
        return;
    }

    if (m_table_model->rowCount() == 0) {
        QMessageBox::warning(this, "Warning", "Selected table is empty.");
    }

    m_description_table->clear();
    QString description_text = m_database_handler->GetTableDescription(table_name);

    if (table_name == "purchases") {
        QSqlQuery sum_query;
        const QString sum_query_string = R"(
            SELECT COALESCE(SUM(c.price), 0) AS total
            FROM purchases p
            JOIN cars c ON c.id = p.car_id
        )";
        if (sum_query.exec(sum_query_string) && sum_query.next()) {
            const qint64 total = sum_query.value("total").toLongLong();
            description_text += "\nTotal sales amount: " + FormatPrice(static_cast<int>(total));
        }
    }

    m_description_table->setText(description_text);
    m_description_table->setWordWrap(true);
    m_description_table->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_description_table->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_data_table->setModel(m_table_model.get());
    m_data_table->resizeColumnsToContents();
    m_data_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_data_table->setSortingEnabled(true);
    m_data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

}

void AdminTableWidget::AddRecord() {
    if (!m_data_table->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем имя таблицы
    QString tableName = m_table_selector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_table_selector->currentText();
    }
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
        // Открываем диалог редактирования для ввода данных
        AdminRecordEditDialog dialog(newRecord, this);
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

void AdminTableWidget::DeleteRecord() {
    if (!m_data_table->model()) {
        QMessageBox::critical(this, "Error", "Select a table before deleting a record.");
        return;
    }

    QString table_name = m_table_selector->currentData().toString();
    if (table_name.isEmpty()) {
        table_name = m_table_selector->currentText();
    }

    const QString col_name = "id";
    int id = -1;

    QItemSelectionModel* selectionModel = m_data_table->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        const QModelIndexList selectedRows = selectionModel->selectedRows();
        if (selectedRows.size() == 1) {
            id = m_data_table->model()->data(m_data_table->model()->index(selectedRows.first().row(), 0)).toInt();
        }
    }

    if (id < 0) {
        const int min_id = m_database_handler->GetMaxOrMinValueFromTable("MIN", col_name, table_name);
        const int max_id = m_database_handler->GetMaxOrMinValueFromTable("MAX", col_name, table_name);

        bool ok = false;
        id = QInputDialog::getInt(
            this,
            tr("Delete Record"),
            tr("Enter record ID:"),
            1,
            min_id,
            max_id,
            1,
            &ok
        );

        if (!ok) {
            return;
        }

        if (id < min_id || id > max_id) {
            QMessageBox::warning(this, "Invalid Data", "The entered ID is out of range.");
            return;
        }
    }

    if (GetConfirmation(table_name, col_name, id)) {
        const QStringList foreignKeys = m_database_handler->GetForeignKeysForColumn(table_name, col_name);

        QString infoMessage;
        if (!foreignKeys.isEmpty()) {
            infoMessage = "Deleting this record affects related tables:\n";
            infoMessage += foreignKeys.join("\n");
            infoMessage += "\n\nDo you want to continue?";
        } else {
            infoMessage = "Do you want to delete this record?";
        }

        if (QMessageBox::warning(this, "Delete Confirmation", infoMessage, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }

        QSqlQuery query;
        const QString query_string = QString("DELETE FROM %1 WHERE %2 = :id").arg(table_name, col_name);

        query.prepare(query_string);
        query.bindValue(":id", id);

        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete record: " + query.lastError().text());
            return;
        }

        LoadTable();
    }
}

void AdminTableWidget::EditRecord() {
    if (!m_data_table->model()) {
        QMessageBox::critical(this, "Error", "Select a table before editing a record.");
        return;
    }

    QString table_name = m_table_selector->currentData().toString();
    if (table_name.isEmpty()) {
        table_name = m_table_selector->currentText();
    }

    const QString primary_key_column = GetPrimaryKeyColumnName(table_name);
    if (primary_key_column.isEmpty()) {
        QMessageBox::critical(this, "Error", "Primary key column was not found for this table.");
        return;
    }

    int id = -1;
    QItemSelectionModel* selectionModel = m_data_table->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        const QModelIndexList selectedRows = selectionModel->selectedRows();
        if (selectedRows.size() == 1) {
            id = m_data_table->model()->data(m_data_table->model()->index(selectedRows.first().row(), 0)).toInt();
        }
    }

    if (id < 0) {
        const int min_id = m_database_handler->GetMaxOrMinValueFromTable("MIN", primary_key_column, table_name);
        const int max_id = m_database_handler->GetMaxOrMinValueFromTable("MAX", primary_key_column, table_name);

        bool ok = false;
        id = QInputDialog::getInt(
            this,
            tr("Edit Record"),
            tr("Enter record ID:"),
            min_id,
            min_id,
            max_id,
            1,
            &ok
        );

        if (!ok) {
            return;
        }
    }

    const QString query_str = QString("SELECT * FROM %1 WHERE %2 = %3")
                                  .arg(table_name)
                                  .arg(primary_key_column)
                                  .arg(id);

    QSqlQuery query;
    if (!query.exec(query_str) || !query.next()) {
        QMessageBox::critical(this, "Error", "Record with specified ID was not found.");
        return;
    }

    QSqlRecord record = query.record();
    for (int i = 0; i < record.count(); i++) {
        record.setValue(i, query.value(i));
    }

    try {
        AdminRecordEditDialog dialog(record, this);
        if (dialog.exec() == QDialog::Accepted) {
            QSqlRecord updatedRecord = dialog.GetUpdatedRecord();

            QStringList setClauses;
            for (int col = 0; col < updatedRecord.count(); ++col) {
                const QString fieldName = updatedRecord.fieldName(col);
                const QString newValue = updatedRecord.value(col).toString();
                setClauses.append(QString("%1 = '%2'").arg(fieldName, newValue));
            }

            const QString updateQuery = QString("UPDATE %1 SET %2 WHERE %3 = %4")
                                            .arg(table_name)
                                            .arg(setClauses.join(", "))
                                            .arg(primary_key_column)
                                            .arg(id);

            if (!m_database_handler->ExecuteQuery(updateQuery)) {
                throw std::runtime_error("Failed to update record.");
            }

            LoadTable();
            QMessageBox::information(this, "Success", "Record updated.");
        }
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

QString AdminTableWidget::GetPrimaryKeyColumnName(const QString& table_name) {
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

bool AdminTableWidget::GetConfirmation(const QString& table_name, const QString& primary_key_column, int id) {
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM %1 WHERE %2 = :id").arg(table_name, primary_key_column));
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(nullptr, "Error", "Failed to fetch record for confirmation: " + query.lastError().text());
        return false;
    }

    const QString message = QString("Delete record with %1 = %2?")
                                .arg(primary_key_column)
                                .arg(id);
    const auto result = QMessageBox::question(
        this,
        "Delete Confirmation",
        message,
        QMessageBox::Yes | QMessageBox::No
    );
    return result == QMessageBox::Yes;
}

bool AdminTableWidget::IsRequestTable(const QString& table_name) const {
    return AdminTableModel::IsRequestTableName(table_name);
}

void AdminTableWidget::ShowRequestButtons(bool show) {
    if (auto approve = m_floating_menu->findChild<QPushButton*>("approve_button"))
        approve->setVisible(show);
    if (auto reject = m_floating_menu->findChild<QPushButton*>("reject_button"))
        reject->setVisible(show);
}

void AdminTableWidget::UpdateRequestStatus(const QString& table_name, const QString& status, const int request_id) {
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

void AdminTableWidget::ApproveRequest()
{
    QModelIndex currentIndex = m_data_table->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::warning(this, "Предупреждение", "Выберите заявку для подтверждения.");
        return;
    }

    int row = currentIndex.row();
    int id = m_data_table->model()->data(m_data_table->model()->index(row, 0)).toInt();
    QString table_name = m_table_selector->currentData().toString();
    if (table_name.isEmpty()) {
        table_name = m_table_selector->currentText();
    }

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

void AdminTableWidget::RejectRequest()
{
    QModelIndex currentIndex = m_data_table->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::warning(this, "Предупреждение", "Выберите заявку для отклонения.");
        return;
    }

    int row = currentIndex.row();
    int id = m_data_table->model()->data(m_data_table->model()->index(row, 0)).toInt();
    QString table_name = m_table_selector->currentData().toString();
    if (table_name.isEmpty()) {
        table_name = m_table_selector->currentText();
    }

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
