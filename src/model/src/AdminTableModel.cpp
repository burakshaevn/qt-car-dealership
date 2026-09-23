#include "AdminTableModel.h"

#include "DatabaseHandler.h"

#include <QSqlError>
#include <QSqlField>
#include <QSqlIndex>

AdminTableModel::AdminTableModel(QSharedPointer<DatabaseHandler> database, QObject* parent)
    : QSqlTableModel(parent, database ? database->database() : QSqlDatabase())
    , m_database(std::move(database))
{
    setEditStrategy(QSqlTableModel::OnManualSubmit);
}

bool AdminTableModel::load(const AdminTableInfo& info)
{
    const QStringList kKnown = database().tables(QSql::AllTables);
    if (!kKnown.contains(info.ViewName) || !kKnown.contains(info.TableName)) {
        qWarning() << "Admin table is not present in the database:" << info.ViewName;
        return false;
    }

    m_info = info;
    setTable(info.ViewName);
    if (!info.isEditable()) {
        // Views are read-only: order by the first column (id) for a stable listing.
        setSort(0, Qt::AscendingOrder);
    }
    return select();
}

QSqlRecord AdminTableModel::blankRecord() const
{
    QSqlRecord record = database().record(m_info.TableName);
    for (int i = 0; i < record.count(); ++i) {
        record.setValue(i, QVariant());
        record.setGenerated(i, true);
    }
    return record;
}

QVariant AdminTableModel::keyAt(const int row) const
{
    return QSqlTableModel::data(index(row, 0), Qt::EditRole);
}

bool AdminTableModel::submitOrRevert(QString* error)
{
    if (submitAll()) {
        return true;
    }
    const QSqlError kError = lastError();
    revertAll();
    if (error) {
        *error = m_database ? m_database->userMessage(kError) : kError.text();
    }
    return false;
}

bool AdminTableModel::insert(const QSqlRecord& record, QString* error)
{
    if (!m_info.isEditable()) {
        return false;
    }
    QSqlRecord toInsert = record;
    // Let SQLite assign INTEGER PRIMARY KEY values when the field is left empty.
    const QSqlIndex kPrimary = primaryKey();
    for (int i = 0; i < kPrimary.count(); ++i) {
        const int kPos = toInsert.indexOf(kPrimary.fieldName(i));
        if (kPos >= 0 && toInsert.value(kPos).toString().isEmpty()) {
            toInsert.setGenerated(kPos, false);
        }
    }
    if (!insertRecord(-1, toInsert)) {
        if (error) {
            *error = m_database ? m_database->userMessage(lastError()) : lastError().text();
        }
        return false;
    }
    return submitOrRevert(error);
}

bool AdminTableModel::update(const int row, const QSqlRecord& record, QString* error)
{
    if (!m_info.isEditable() || !setRecord(row, record)) {
        if (error && m_database) {
            *error = m_database->userMessage(lastError());
        }
        return false;
    }
    return submitOrRevert(error);
}

bool AdminTableModel::remove(const int row, QString* error)
{
    if (!m_info.isEditable() || !removeRow(row)) {
        if (error && m_database) {
            *error = m_database->userMessage(lastError());
        }
        return false;
    }
    return submitOrRevert(error);
}

QVariant AdminTableModel::data(const QModelIndex& index, const int role) const
{
    if (role == Qt::TextAlignmentRole) {
        const QVariant kValue = QSqlTableModel::data(index, Qt::EditRole);
        const auto kType = kValue.metaType().id();
        if (kType == QMetaType::Int || kType == QMetaType::LongLong || kType == QMetaType::Double) {
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QSqlTableModel::data(index, role);
}
