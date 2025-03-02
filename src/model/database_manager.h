#pragma once

#ifndef DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <utility>
#include <QMessageBox>

#include "domain.h"
#include "svg.h"

class DatabaseManager {
public:
    DatabaseManager();

    template <typename Container, typename Port>
    DatabaseManager(Container&& host, Port&& port, Container&& db_name, Container&& username, Container&& password)
    {
        db_ = QSqlDatabase::addDatabase("QPSQL");
        db_.setHostName(std::forward<Container>(host));
        db_.setPort(std::forward<Port>(port));
        db_.setDatabaseName(std::forward<Container>(db_name));
        db_.setUserName(std::forward<Container>(username));
        db_.setPassword(std::forward<Container>(password));
    }

    bool Open();
    void Close();

    void UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password);

    bool StopIsExists(const QString& name);
    bool AddStop(const QString& name, double latitude, double longitude);
    bool DeleteStop(const Stop* stop);
    std::optional<Stop> FindStop(const QStringView name);
    bool UpdateStop(const QStringView old_name, const QStringView new_name, const double latitude, const double longitude);

    bool AddDistance(const QString& from, const QString& to, int distance);
    bool DeleteDistance(const QString& from, const QString& to);
    bool DistanceIsExists(const QString& from, const QString& to);
    int GetDistance(const Stop* from, const Stop* to);

    bool BusExists(const QString& name);
    bool AddBus(const Bus* bus);
    bool DeleteBus(const Bus* bus);
    std::optional<Bus> FindBus(const QStringView name);
    bool UpdateBus(const Bus* bus);

    std::optional<svg::Color> FindColor(const size_t color_index);

    // ѕроверка существовани€ св€зки bus_stop
    bool BusStopIsExists(int bus_id, const QString& stop_name);

    bool AddBusStop(int bus_id, const QString& stop_name);
    bool AddBusStop(int bus_id, int stop_id, int stop_position, const QString& stop_name);

    bool UpdateRoutingSettings(const double bus_velocity, const int bus_wait_time);

    // ¬ыполнение запроса без возврата данных
    void ExecuteQuery(const QString& queryStr);

    // ¬ыполнение запроса с возвратом данных
    QSqlQuery ExecuteSelectQuery(const QString& queryStr);

    const int GetRowsCount(QStringView table_name) const;

private:
    QSqlDatabase db_;
};

#endif // DATABASEMANAGER_H
