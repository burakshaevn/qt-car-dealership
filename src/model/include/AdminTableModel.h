#pragma once

#ifndef ADMINTABLEMODEL_H
#define ADMINTABLEMODEL_H

#include <QSqlQueryModel>
#include <QString>

class AdminTableModel final : public QSqlQueryModel
{
    Q_OBJECT

public:
    explicit AdminTableModel(QObject* parent = nullptr);

    bool load(const QString& tableName);
    QString getCurrentTableName() const;

    static bool isRequestTableName(const QString& tableName);

private:
    static QString buildSelectQuery(const QString& tableName);

private:
    QString m_currentTableName;
};

#endif // ADMINTABLEMODEL_H
