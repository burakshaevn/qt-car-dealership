#pragma once

#ifndef RECORD_EDIT_DIALOG_H
#define RECORD_EDIT_DIALOG_H

#include <QHash>
#include <QList>
#include <QSqlRecord>

#include "UiKit.h"

class QLineEdit;

/*!
 * \brief Generic form for a database record (admin panel).
 *
 * One input per field; primary key fields are read-only when editing.
 * Empty inputs become SQL NULL, so NOT NULL/CHECK constraints are enforced by
 * the database and reported through DatabaseHandler::userMessage().
 */
class RecordEditDialog final : public FormDialog
{
    Q_OBJECT
public:
    RecordEditDialog(const QString& title,
                     const QSqlRecord& record,
                     const QStringList& readOnlyFields,
                     const QHash<QString, QString>& labels = {},
                     QWidget* parent = nullptr);

    [[nodiscard]] QSqlRecord record() const;

private:
    QSqlRecord m_record;
    QList<QLineEdit*> m_inputs;
};

#endif // RECORD_EDIT_DIALOG_H
