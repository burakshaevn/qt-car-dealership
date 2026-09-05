#include "../include/AdminTableWidget.h"
#include "AdminRecordEditDialog.h"
#include "ThemeStyleProvider.h"
#include <QWidget>
#include <QString>

AdminTableWidget::AdminTableWidget(QSharedPointer<DatabaseHandler> dbManager, QWidget* parent)
    : QWidget(parent)
    , m_databaseHandler(std::move(dbManager))
    , m_dataTable(new QTableView(this))
    , m_descriptionTable(new QLabel(this))
{
    // Настройка отображения таблицы
    m_dataTable->verticalHeader()->hide();      // Скрываем номера строк
    m_dataTable->setAlternatingRowColors(true); // Чередующиеся цвета строк
    m_dataTable->setSortingEnabled(true);       // Включаем сортировку

    // Настройка заголовков таблицы
    QHeaderView* header = m_dataTable->horizontalHeader();
    header->setSortIndicatorShown(true);         // Показываем индикатор сортировки
    header->setSectionsClickable(true);          // Разрешаем клик по заголовкам
}

void AdminTableWidget::buildAdminTables()
{
    m_tableSelector = new QComboBox(this);
    auto* layout = new QVBoxLayout(this);
    
    QSqlQuery tablesQuery;
    if (tablesQuery.exec("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public' ORDER BY table_name")) {
        while (tablesQuery.next()) {
            const QString kTableName = tablesQuery.value(0).toString();
            m_tableSelector->addItem(kTableName, kTableName);
        }
    }

    m_tableSelector->setCurrentIndex(-1);
    applyThemeStyle(m_tableSelector, "AdminTableSelector");
    layout->addWidget(m_tableSelector);
    applyThemeStyle(m_descriptionTable, "AdminDescription");
    layout->addWidget(m_descriptionTable);

    // Таблица для отображения данных
    m_dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dataTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dataTable->horizontalHeader()->setStretchLastSection(true);
    m_dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    applyThemeStyle(m_dataTable, "AdminTableView");
    layout->addWidget(m_dataTable);

    m_floatingMenu.reset(new QWidget(this));
    applyThemeStyle(m_floatingMenu.get(), "AdminFloatingMenu");
    m_floatingMenu->setFixedSize(335, 74);

    // Горизонтальное размещение кнопок в меню
    QHBoxLayout* menuLayout = new QHBoxLayout(m_floatingMenu.get());
    menuLayout->setContentsMargins(20, 10, 20, 10);
    menuLayout->setSpacing(20);

    QPushButton* addButton;     // Кнопка добавления записи
    QPushButton* deleteButton;  // Кнопка удаления записи
    QPushButton* editButton;    // Кнопка редактирования записи
    QPushButton* logoutButton;  // Кнопка выхода из системы
    QPushButton* approveButton; // Кнопка подтверждения заявки
    QPushButton* rejectButton;  // Кнопка отклонения заявки

    addButton = new QPushButton("", m_floatingMenu.get());
    applyThemeIcon(addButton, "add.svg");
    addButton->setIconSize(QSize(35, 35));
    applyThemeStyle(addButton, "IconButton");

    editButton = new QPushButton("", m_floatingMenu.get());
    applyThemeIcon(editButton, "edit.svg");
    editButton->setIconSize(QSize(35, 35));
    applyThemeStyle(editButton, "IconButton");

    deleteButton = new QPushButton("", m_floatingMenu.get());
    applyThemeIcon(deleteButton, "delete.svg");
    deleteButton->setIconSize(QSize(35, 35));
    applyThemeStyle(deleteButton, "IconButton");

    logoutButton = new QPushButton("", m_floatingMenu.get());
    applyThemeIcon(logoutButton, "navigate_next.svg");
    logoutButton->setIconSize(QSize(35, 35));
    applyThemeStyle(logoutButton, "IconButton");

    // Создаем кнопки для обработки заявок
    approveButton = new QPushButton("", m_floatingMenu.get());
    applyThemeIcon(approveButton, "check.svg");
    approveButton->setIconSize(QSize(35, 35));
    applyThemeStyle(approveButton, "IconButton");
    approveButton->setToolTip("Подтвердить заявку");

    rejectButton = new QPushButton("", m_floatingMenu.get());
    applyThemeIcon(rejectButton, "close.svg");
    rejectButton->setIconSize(QSize(35, 35));
    applyThemeStyle(rejectButton, "IconButton");
    rejectButton->setToolTip("Отклонить заявку");

    menuLayout->addWidget(approveButton);
    menuLayout->addWidget(rejectButton);
    menuLayout->addWidget(addButton);
    menuLayout->addWidget(editButton);
    menuLayout->addWidget(deleteButton);
    menuLayout->addWidget(logoutButton);

    // Скрываем кнопки обработки заявок по умолчанию
    showRequestButtons(false);

    // Установим позицию меню (по центру внизу)
    m_floatingMenu->move(378, 460);
    m_floatingMenu->show();

    connect(m_tableSelector, &QComboBox::currentTextChanged, this, &AdminTableWidget::loadTable);
    connect(addButton, &QPushButton::clicked, this, &AdminTableWidget::addRecord);
    connect(editButton, &QPushButton::clicked, this, &AdminTableWidget::editRecord);
    connect(deleteButton, &QPushButton::clicked, this, &AdminTableWidget::deleteRecord);
    connect(logoutButton, &QPushButton::clicked, this, &AdminTableWidget::logout);
    connect(approveButton, &QPushButton::clicked, this, &AdminTableWidget::approveRequest);
    connect(rejectButton, &QPushButton::clicked, this, &AdminTableWidget::rejectRequest);

    m_floatingMenu->installEventFilter(this);
}

bool AdminTableWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_floatingMenu.get()) {
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
                QPoint newPos = m_floatingMenu->pos() + mouseEvent->pos() - dragStartPos;

                // Убедитесь, что новое положение находится в пределах родительского окна
                QWidget* parent = m_floatingMenu->parentWidget();
                if (parent) {
                    QRect parentRect = parent->rect();
                    QSize menuSize = m_floatingMenu->size();

                    newPos.setX(std::max(0, std::min(newPos.x(), parentRect.width() - menuSize.width())));
                    newPos.setY(std::max(0, std::min(newPos.y(), parentRect.height() - menuSize.height())));
                }

                m_floatingMenu->move(newPos);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            dragging = false; // Завершаем перетаскивание
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void AdminTableWidget::loadTable()
{
    QString tableName = m_tableSelector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_tableSelector->currentText();
    }

    showRequestButtons(AdminTableModel::isRequestTableName(tableName));

    if (!m_tableModel) {
        m_tableModel.reset(new AdminTableModel(this));
    }

    if (!m_tableModel->load(tableName)) {
        QMessageBox::critical(this, "Error", "Query execution failed.");
        return;
    }

    if (m_tableModel->rowCount() == 0) {
        QMessageBox::warning(this, "Warning", "Selected table is empty.");
    }

    m_descriptionTable->clear();
    QString descriptionText = m_databaseHandler->getTableDescription(tableName);

    if (tableName == "purchases") {
        QSqlQuery sumQuery;
        const QString kSumQueryString = R"(
            SELECT COALESCE(SUM(c.price), 0) AS total
            FROM purchases p
            JOIN cars c ON c.id = p.car_id
        )";
        if (sumQuery.exec(kSumQueryString) && sumQuery.next()) {
            const qint64 kTotal = sumQuery.value("total").toLongLong();
            descriptionText += "\nTotal sales amount: " + formatPrice(static_cast<int>(kTotal));
        }
    }

    m_descriptionTable->setText(descriptionText);
    m_descriptionTable->setWordWrap(true);
    m_descriptionTable->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_descriptionTable->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_dataTable->setModel(m_tableModel.get());
    m_dataTable->resizeColumnsToContents();
    m_dataTable->setEditTriggers(QAbstractItemView::DoubleClicked
                                 | QAbstractItemView::SelectedClicked);
    m_dataTable->setSortingEnabled(true);
    m_dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void AdminTableWidget::addRecord()
{
    if (!m_dataTable->model()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем имя таблицы
    QString tableName = m_tableSelector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_tableSelector->currentText();
    }
    if (tableName.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Выберите таблицу для добавления записи.");
        return;
    }

    // Получаем модель и создаём пустую запись
    QAbstractItemModel* model = m_dataTable->model();
    QSqlRecord newRecord;

    for (int col = 0; col < model->columnCount(); ++col) {
        QString fieldName = model->headerData(col, Qt::Horizontal).toString();
        QSqlField field(fieldName, QVariant::String);

        if (fieldName == "id") {
            // Автоматически вычисляем id
            int newId = m_databaseHandler->getMaxOrMinValueFromTable("MAX", fieldName, tableName)
                        + 1;
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
            updatedRecord = dialog.getUpdatedRecord();

            // Формируем SQL-запрос для вставки данных
            QStringList fieldNames, fieldValues;
            for (int col = 0; col < updatedRecord.count(); ++col) {
                QString fieldName = updatedRecord.fieldName(col);
                QString fieldValue = updatedRecord.value(col).toString();

                fieldNames.append(fieldName);
                fieldValues.append("'" + fieldValue + "'");
            }

            QString insertQuery = QString("INSERT INTO %1 (%2) VALUES (%3)")
                                      .arg(tableName, fieldNames.join(", "), fieldValues.join(", "));

            // Выполняем запрос
            QSqlQuery query;
            if (!query.exec(insertQuery)) {
                throw std::runtime_error("");
            }

            // Перезагружаем таблицу
            loadTable();
            QMessageBox::information(this, "Информация", "Новая запись добавлена .");
        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void AdminTableWidget::deleteRecord()
{
    if (!m_dataTable->model()) {
        QMessageBox::critical(this, "Error", "Select a table before deleting a record.");
        return;
    }

    QString tableName = m_tableSelector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_tableSelector->currentText();
    }

    const QString kColName = "id";
    int id = -1;

    QItemSelectionModel* selectionModel = m_dataTable->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        const QModelIndexList kSelectedRows = selectionModel->selectedRows();
        if (kSelectedRows.size() == 1) {
            id = m_dataTable->model()
                     ->data(m_dataTable->model()->index(kSelectedRows.first().row(), 0))
                     .toInt();
        }
    }

    if (id < 0) {
        const int kMinId = m_databaseHandler->getMaxOrMinValueFromTable("MIN", kColName, tableName);
        const int kMaxId = m_databaseHandler->getMaxOrMinValueFromTable("MAX", kColName, tableName);

        bool ok = false;
        id = QInputDialog::getInt(
            this,
            tr("Delete Record"),
            tr("Enter record ID:"),
            1,
            kMinId,
            kMaxId,
            1,
            &ok
        );

        if (!ok) {
            return;
        }

        if (id < kMinId || id > kMaxId) {
            QMessageBox::warning(this, "Invalid Data", "The entered ID is out of range.");
            return;
        }
    }

    if (getConfirmation(tableName, kColName, id)) {
        const QStringList kForeignKeys = m_databaseHandler->getForeignKeysForColumn(tableName,
                                                                                    kColName);

        QString infoMessage;
        if (!kForeignKeys.isEmpty()) {
            infoMessage = "Deleting this record affects related tables:\n";
            infoMessage += kForeignKeys.join("\n");
            infoMessage += "\n\nDo you want to continue?";
        } else {
            infoMessage = "Do you want to delete this record?";
        }

        if (QMessageBox::warning(this, "Delete Confirmation", infoMessage, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }

        QSqlQuery query;
        const QString kQueryString = QString("DELETE FROM %1 WHERE %2 = :id").arg(tableName, kColName);

        query.prepare(kQueryString);
        query.bindValue(":id", id);

        if (!query.exec()) {
            QMessageBox::critical(this, "Error", "Failed to delete record.");
            return;
        }

        loadTable();
    }
}

void AdminTableWidget::editRecord()
{
    if (!m_dataTable->model()) {
        QMessageBox::critical(this, "Error", "Select a table before editing a record.");
        return;
    }

    QString tableName = m_tableSelector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_tableSelector->currentText();
    }

    const QString kPrimaryKeyColumn = getPrimaryKeyColumnName(tableName);
    if (kPrimaryKeyColumn.isEmpty()) {
        QMessageBox::critical(this, "Error", "Primary key column was not found for this table.");
        return;
    }

    int id = -1;
    QItemSelectionModel* selectionModel = m_dataTable->selectionModel();
    if (selectionModel && selectionModel->hasSelection()) {
        const QModelIndexList kSelectedRows = selectionModel->selectedRows();
        if (kSelectedRows.size() == 1) {
            id = m_dataTable->model()
                     ->data(m_dataTable->model()->index(kSelectedRows.first().row(), 0))
                     .toInt();
        }
    }

    if (id < 0) {
        const int kMinId = m_databaseHandler->getMaxOrMinValueFromTable("MIN",
                                                                        kPrimaryKeyColumn,
                                                                        tableName);
        const int kMaxId = m_databaseHandler->getMaxOrMinValueFromTable("MAX",
                                                                        kPrimaryKeyColumn,
                                                                        tableName);

        bool ok = false;
        id = QInputDialog::getInt(
            this,
            tr("Edit Record"),
            tr("Enter record ID:"),
            kMinId,
            kMinId,
            kMaxId,
            1,
            &ok
        );

        if (!ok) {
            return;
        }
    }

    const QString kQueryStr
        = QString("SELECT * FROM %1 WHERE %2 = %3").arg(tableName).arg(kPrimaryKeyColumn).arg(id);

    QSqlQuery query;
    if (!query.exec(kQueryStr) || !query.next()) {
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
            QSqlRecord updatedRecord = dialog.getUpdatedRecord();

            QStringList setClauses;
            for (int col = 0; col < updatedRecord.count(); ++col) {
                const QString kFieldName = updatedRecord.fieldName(col);
                const QString kNewValue = updatedRecord.value(col).toString();
                setClauses.append(QString("%1 = '%2'").arg(kFieldName, kNewValue));
            }

            const QString kUpdateQuery = QString("UPDATE %1 SET %2 WHERE %3 = %4")
                                            .arg(tableName)
                                            .arg(setClauses.join(", "))
                                            .arg(kPrimaryKeyColumn)
                                            .arg(id);

            if (!m_databaseHandler->executeQuery(kUpdateQuery)) {
                throw std::runtime_error("Failed to update record.");
            }

            loadTable();
            QMessageBox::information(this, "Success", "Record updated.");
        }
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

QString AdminTableWidget::getPrimaryKeyColumnName(const QString& tableName)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT a.attname
        FROM pg_index i
        JOIN pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey)
        WHERE i.indrelid = :table_name::regclass AND i.indisprimary;
    )");
    query.bindValue(":table_name", tableName);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

bool AdminTableWidget::getConfirmation(const QString& tableName,
                                       const QString& primaryKeyColumn,
                                       int id)
{
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM %1 WHERE %2 = :id").arg(tableName, primaryKeyColumn));
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(nullptr, "Error", "Failed to fetch record for confirmation. ");
        return false;
    }

    const QString kMessage = QString("Delete record with %1 = %2?")
                                .arg(primaryKeyColumn)
                                .arg(id);
    const auto kResult = QMessageBox::question(
        this,
        "Delete Confirmation",
        kMessage,
        QMessageBox::Yes | QMessageBox::No
    );
    return kResult == QMessageBox::Yes;
}

bool AdminTableWidget::isRequestTable(const QString& tableName) const
{
    return AdminTableModel::isRequestTableName(tableName);
}

void AdminTableWidget::showRequestButtons(bool show)
{
    if (auto approve = m_floatingMenu->findChild<QPushButton*>("approve_button"))
        approve->setVisible(show);
    if (auto reject = m_floatingMenu->findChild<QPushButton*>("reject_button"))
        reject->setVisible(show);
}

void AdminTableWidget::updateRequestStatus(const QString& tableName,
                                           const QString& status,
                                           const int kRequestId)
{
    QString query = QString("UPDATE %1 SET status = '%2', notification_shown = false WHERE id = %3")
        .arg(tableName)
        .arg(status)
        .arg(kRequestId);

    qDebug() << "Executing update query:" << query;

    QVariant result = m_databaseHandler->executeQuery(query);
    if (!result.toBool()) {
        QString error = m_databaseHandler->getLastError();
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус заявки: " + error);
        qDebug() << "Update failed:" << error;
        return;
    }

    loadTable(); // Перезагружаем таблицу для отображения изменений
}

void AdminTableWidget::approveRequest()
{
    QModelIndex currentIndex = m_dataTable->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::warning(this, "Предупреждение", "Выберите заявку для подтверждения.");
        return;
    }

    int row = currentIndex.row();
    int id = m_dataTable->model()->data(m_dataTable->model()->index(row, 0)).toInt();
    QString tableName = m_tableSelector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_tableSelector->currentText();
    }

    // Используем правильные статусы в зависимости от типа таблицы
    QString status;
    if (tableName == "service_requests") {
        status = "подтверждено";
    }
    else {
        status = "одобрено"; // Для loan_requests, insurance_requests и других
    }

    updateRequestStatus(tableName, status, id);
}

void AdminTableWidget::rejectRequest()
{
    QModelIndex currentIndex = m_dataTable->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::warning(this, "Предупреждение", "Выберите заявку для отклонения.");
        return;
    }

    int row = currentIndex.row();
    int id = m_dataTable->model()->data(m_dataTable->model()->index(row, 0)).toInt();
    QString tableName = m_tableSelector->currentData().toString();
    if (tableName.isEmpty()) {
        tableName = m_tableSelector->currentText();
    }

    // Используем правильные статусы в зависимости от типа таблицы
    QString status;
    if (tableName == "service_requests") {
        status = "отменено";
    }
    else {
        status = "отклонено"; // Для loan_requests, insurance_requests и других
    }

    updateRequestStatus(tableName, status, id);
}
