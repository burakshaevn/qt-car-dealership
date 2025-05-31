#pragma once

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QInputDialog>
#include <QSqlField>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QScrollArea>
#include <QMouseEvent>

#include "database_handler.h"
#include "edit_dialog.h"
#include "domain.h"
#include "user.h"

class Table : public QWidget {
    Q_OBJECT

protected:
    Tables current_table_;
    std::shared_ptr<DatabaseHandler> db_manager_;
    QTableView* data_table_;
    QLabel* description_table;

    QComboBox* table_selector_;
    QHBoxLayout* button_layout;
    QPushButton* add_button_;
    QPushButton* delete_button_;
    QPushButton* edit_button_;
    QPushButton* logout_button_;
    QPushButton* approve_button_;
    QPushButton* reject_button_;
    QPushButton* complete_button_;

    std::unique_ptr<QWidget> floating_menu_;

    void UpdateRequestStatus(const QString& table_name, int request_id, const QString& new_status);
    bool IsRequestTable(const QString& table_name) const;
    void ShowRequestButtons(bool show);

public:
    explicit Table(std::shared_ptr<DatabaseHandler> db_manager, const User* user, QWidget* parent = nullptr);

    void BuildAdminTables();

    void LoadTable();

    void AddRecord();
    void DeleteRecord();
    void EditRecord();

    QString GetPrimaryKeyColumnName(const QString& table_name);

    bool GetConfirmation(const QString& table_name, const QString& primary_key_column, int id);

    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void ApproveRequest();
    void RejectRequest();
    void CompleteRequest();

signals:
    void Logout();
};

#endif // TABLE_MANAGER_H
