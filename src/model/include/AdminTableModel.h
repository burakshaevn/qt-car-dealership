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

    bool Load(const QString& table_name);
    QString GetCurrentTableName() const;

    static bool IsRequestTableName(const QString& table_name);

private:
    static QString BuildSelectQuery(const QString& table_name);

private:
    QString current_table_name_;
};

#endif // ADMINTABLEMODEL_H
