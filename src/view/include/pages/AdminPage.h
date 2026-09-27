#pragma once

#ifndef ADMIN_PAGE_H
#define ADMIN_PAGE_H

#include <QWidget>

class QAbstractItemModel;
class QLabel;
class QLineEdit;
class QPushButton;
class QSortFilterProxyModel;
class QTableView;

/*!
 * \brief Admin data browser: title/description, toolbar (search, approve/reject,
 *        add/edit/delete) and a sortable, filterable table.
 */
class AdminPage final : public QWidget
{
    Q_OBJECT
public:
    explicit AdminPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    void setHeader(const QString& title, const QString& description, const QString& summary = {});
    void setRequestMode(bool isRequestTable);
    void setEditable(bool editable);

    /// Source-model row of the current selection or -1.
    [[nodiscard]] int currentSourceRow() const;

signals:
    void approveRequested();
    void rejectRequested();
    void addRequested();
    void editRequested();
    void deleteRequested();

private:
    QLabel* m_title = nullptr;
    QLabel* m_description = nullptr;
    QLabel* m_summary = nullptr;
    QLineEdit* m_search = nullptr;
    QPushButton* m_approve = nullptr;
    QPushButton* m_reject = nullptr;
    QPushButton* m_add = nullptr;
    QPushButton* m_edit = nullptr;
    QPushButton* m_delete = nullptr;
    QTableView* m_table = nullptr;
    QSortFilterProxyModel* m_proxy = nullptr;
};

#endif // ADMIN_PAGE_H
