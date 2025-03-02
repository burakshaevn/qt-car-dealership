#include "database_manager.h"

DatabaseManager::DatabaseManager() = default;

bool DatabaseManager::Open() {
    return db_.open();
}

void DatabaseManager::Close() {
    db_.close();
}

void DatabaseManager::UpdateConnection(const QString& host, int port, const QString& db_name, const QString& username, const QString& password) {
    db_ = QSqlDatabase::addDatabase("QPSQL");
    db_.setHostName(host);
    db_.setPort(port);
    db_.setDatabaseName(db_name);
    db_.setUserName(username);
    db_.setPassword(password);
}

bool DatabaseManager::StopIsExists(const QString& name) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM stops WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    else{
        // qDebug() << Q_FUNC_INFO  << query.lastError().text();
    }
    return false;
}
bool DatabaseManager::AddStop(const QString& name, double latitude, double longitude)
{
    if (StopIsExists(name)) {
        // qDebug() << Q_FUNC_INFO  << "Остановка уже существует:" << name;
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO stops (name, latitude, longitude) VALUES (:name, :latitude, :longitude)");
    query.bindValue(":name", name);
    query.bindValue(":latitude", latitude);
    query.bindValue(":longitude", longitude);


    if (query.exec()) {
        return true;
    }
    else {
        QSqlQuery resetQuery;
        resetQuery.exec("SELECT setval('stops_id_seq', (SELECT COALESCE(MAX(id), 1) FROM stops), false)");
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
        return false;
    }
}
bool DatabaseManager::DeleteStop(const Stop* stop) {
    QSqlQuery deleteStopQuery;
    deleteStopQuery.prepare("DELETE FROM stops WHERE name = :name");
    deleteStopQuery.bindValue(":name", stop->name);

    if (!deleteStopQuery.exec()) {
        //qDebug() << Q_FUNC_INFO << "Ошибка при удалении остановки:" << deleteStopQuery.lastError().text();
        return false;
    }

    return true;
}
std::optional<Stop> DatabaseManager::FindStop(const QStringView name)
{
    QSqlQuery query = ExecuteSelectQuery(
        QString("SELECT * FROM stops WHERE name = '%1'").arg(name)
    );

    if (query.next()) {
        Stop stop;
        stop.name = query.value("name").toString();
        stop.coords.lat = query.value("latitude").toDouble();
        stop.coords.lng = query.value("longitude").toDouble();
        return stop;
    }

    return std::nullopt;
}
bool DatabaseManager::UpdateStop(const QStringView old_name, const QStringView new_name, const double latitude, const double longitude) {
    QSqlQuery query;
    query.prepare("UPDATE public.stops "
                  "SET name = :new_name, latitude = :lat, longitude = :lon "
                  "WHERE stops.name = :old_name;");
    query.bindValue(":new_name", new_name.toString());
    query.bindValue(":lat", latitude);
    query.bindValue(":lon", longitude);
    query.bindValue(":old_name", old_name.toString());

    if (!query.exec()) {
        qWarning() << "Не удалось выполнить запрос:" << query.lastError().text();
        return false;
    }

    return true;
}
// Проверка существования дистанции
bool DatabaseManager::DistanceIsExists(const QString& from_stop, const QString& to_stop) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM distances d "
                  "JOIN stops s1 ON d.from_stop_id = s1.id "
                  "JOIN stops s2 ON d.to_stop_id = s2.id "
                  "WHERE s1.name = :from_stop AND s2.name = :to_stop");
    query.bindValue(":from_stop", from_stop);
    query.bindValue(":to_stop", to_stop);

    if (!query.exec()) {
        qWarning() << "Failed to execute query:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

int DatabaseManager::GetDistance(const Stop* from, const Stop* to)
{
    QSqlQuery query = ExecuteSelectQuery(
        QString("SELECT d.id, s1.name AS from_stop_name, s2.name AS to_stop_name, d.distance FROM distances d JOIN stops s1 ON d.from_stop_id = s1.id JOIN stops s2 ON d.to_stop_id = s2.id WHERE s1.name = '%1' AND s2.name = '%2';")
        .arg(from->name)
        .arg(to->name)
    );
    if (query.next()) {
        return query.value("distance").toInt();
    }
    else {
        QSqlQuery new_query = ExecuteSelectQuery(
            QString("SELECT d.id, s1.name AS from_stop_name, s2.name AS to_stop_name, d.distance FROM distances d JOIN stops s1 ON d.from_stop_id = s1.id JOIN stops s2 ON d.to_stop_id = s2.id WHERE s1.name = '%1' AND s2.name = '%2';")
            .arg(to->name)
            .arg(from->name)
        );
        if (new_query.next()) {
            return new_query.value("distance").toInt();
        }
        else{
            // qDebug() << Q_FUNC_INFO << query.lastError().text();
        }
    }
    return 0;
}

//#include <random>
//
//template <typename T>
//T GetRandomNum(const T min, const T max) {
//    // Ensure random engine is seeded once by making it static
//    static std::mt19937 engine{ std::random_device{}() };
//    std::uniform_int_distribution<T> distribution(min, max);
//    return distribution(engine);
//}

//void DatabaseManager::FillDistances() {
//    QSqlQuery stop_query;
//    stop_query.prepare("SELECT id FROM stops");
//
//    if (!stop_query.exec()) {
//        // qDebug() << "Failed to execute stop query:" << stop_query.lastError();
//        return;
//    }
//
//    QVector<int> stop_ids;
//    while (stop_query.next()) {
//        stop_ids.append(stop_query.value("id").toInt());
//    }
//
//    for (int i = 0; i < stop_ids.size(); ++i) {
//        for (int j = i + 1; j < stop_ids.size(); ++j) {
//            int distance = GetRandomNum(900, 3000);
//
//            QSqlQuery query;
//            query.prepare("INSERT INTO public.distances (from_stop_id, to_stop_id, distance) VALUES (:from_stop_id, :to_stop_id, :distance)");
//            query.bindValue(":from_stop_id", stop_ids[i]);
//            query.bindValue(":to_stop_id", stop_ids[j]);
//            query.bindValue(":distance", distance);
//
//            if (!query.exec()) {
//                // qDebug() << "Insert distance failed:" << query.lastError();
//            }
//        }
//    }
//}

//std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher> DatabaseManager::GetDistances()
//{
//    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher> distances;
//
//    QSqlQuery query = ExecuteSelectQuery("SELECT from_stop, to_stop, distance FROM distances;");
//    while (query.next()) {
//        QString from_stop_name = query.value("from_stop").toString();
//        QString to_stop_name = query.value("to_stop").toString();
//        int distance = query.value("distance").toInt();
//        auto from_stop_opt = FindStop(from_stop_name);
//        auto to_stop_opt = FindStop(to_stop_name);
//
//        if (from_stop_opt.has_value() && to_stop_opt.has_value()) {
//            const Stop* from_stop = &from_stop_opt.value();
//            const Stop* to_stop = &to_stop_opt.value();
//            distances[{from_stop, to_stop}] = distance;
//        }
//    }
//    return distances;
//}

bool DatabaseManager::AddDistance(const QString& from, const QString& to, int distance) {
    if (DistanceIsExists(from, to)) {
        // qDebug() << Q_FUNC_INFO << "Дистанция уже существует:" << from << "->" << to;
        return false;
    }

    QSqlQuery query;

    query.prepare("SELECT id FROM stops WHERE name = :from_name");
    query.bindValue(":from_name", from);
    if (!query.exec() || !query.next()) {
        // qDebug() << Q_FUNC_INFO << "Не удалось получить ID для остановки:" << from;
        return false;
    }
    int from_stop_id = query.value(0).toInt();

    query.prepare("SELECT id FROM stops WHERE name = :to_name");
    query.bindValue(":to_name", to);
    if (!query.exec() || !query.next()) {
        // qDebug() << Q_FUNC_INFO << "Не удалось получить ID для остановки:" << to;
        return false;
    }
    int to_stop_id = query.value(0).toInt();

    query.prepare("INSERT INTO distances (from_stop_id, to_stop_id, distance) VALUES (:from_id, :to_id, :distance)");
    query.bindValue(":from_id", from_stop_id);
    query.bindValue(":to_id", to_stop_id);
    query.bindValue(":distance", distance);

    if (query.exec()) {
        return true;
    } else {
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
        return false;
    }
}

bool DatabaseManager::DeleteDistance(const QString& from, const QString& to){
    if (DistanceIsExists(from, to)) {
        // qDebug() << Q_FUNC_INFO << "Дистанция уже существует:" << from << "->" << to;
        return false;
    }

    QSqlQuery query;

    query.prepare("DELETE FROM distances d "
                                     "USING stops s1, stops s2"
                                     "WHERE d.from_stop_id = s1.id "
                                     "AND d.to_stop_id = s2.id"
                                     "AND s1.name = ':from_stop' "
                                     "AND s2.name = ':to_stop';");
    query.bindValue(":from_stop", from);
    query.bindValue(":to_stop", to);
    if (!query.exec() || !query.next()) {
        // qDebug() << Q_FUNC_INFO << "Не удалось выполнить удаление: " << from;
        return false;
    }
    return true;
}

// Проверка существования автобуса
bool DatabaseManager::BusExists(const QString& name) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM buses WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool DatabaseManager::AddBus(const Bus* bus)
{
    if (BusExists(bus->name)) {
        // qDebug() << Q_FUNC_INFO << "Автобус уже существует: " << bus->name;
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO buses (name, is_roundtrip, color_index, bus_type, capacity, has_wifi, has_sockets, is_night, is_available, price, is_day) "
                  "VALUES (:name, :is_roundtrip, :color_index, :bus_type, :capacity, :has_wifi, :has_sockets, :is_night, :is_available, :price, :is_day) "
                  "RETURNING id");
    query.bindValue(":name", bus->name);
    query.bindValue(":is_roundtrip", bus->is_roundtrip);
    query.bindValue(":color_index", bus->color_index);
    query.bindValue(":bus_type", BusTypeToString(bus->bus_type));
    query.bindValue(":capacity", bus->capacity);
    query.bindValue(":has_wifi", bus->has_wifi);
    query.bindValue(":has_sockets", bus->has_sockets);
    query.bindValue(":is_night", bus->is_night);
    query.bindValue(":is_available", bus->is_available);
    query.bindValue(":price", bus->price);
    query.bindValue(":is_day", bus->is_day);

    if(!query.exec()){
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
        return false;
    }

    int bus_id = 0;
    if (query.next()) {
        bus_id = query.value(0).toInt();
    }
    for (auto stop : bus->stops) {
        if (!AddBusStop(bus_id, stop->name)) {
            return false;
        }
    }

    return true;
}
bool DatabaseManager::UpdateBus(const Bus* bus) {
    // Начало транзакции
    if (!QSqlDatabase::database().transaction()) {
        // qDebug() <<  Q_FUNC_INFO << "Не удалось начать транзакцию:" << QSqlDatabase::database().lastError().text();
        return false;
    }

    // Удаление существующих записей
    QSqlQuery deleteQuery;
    deleteQuery.prepare("DELETE FROM bus_stops WHERE bus_id = (SELECT id FROM buses WHERE name = :name)");
    deleteQuery.bindValue(":name", bus->name);
    if (!deleteQuery.exec()) {
        // qDebug() <<  Q_FUNC_INFO << "Ошибка удаления остановок:" << deleteQuery.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    // Обновление данных автобуса
    if (BusExists(bus->name)) {
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE buses SET is_roundtrip = :is_roundtrip, color_index = :color_index, bus_type = :bus_type, capacity = :capacity, has_wifi = :has_wifi, has_sockets = :has_sockets, is_night = :is_night, is_day = :is_day, is_available = :is_available, price = :price WHERE name = :name");
        updateQuery.bindValue(":name", bus->name);
        updateQuery.bindValue(":is_roundtrip", bus->is_roundtrip);
        updateQuery.bindValue(":color_index", bus->color_index);
        updateQuery.bindValue(":bus_type", BusTypeToString(bus->bus_type));
        updateQuery.bindValue(":capacity", bus->capacity);
        updateQuery.bindValue(":has_wifi", bus->has_wifi);
        updateQuery.bindValue(":has_sockets", bus->has_sockets);
        updateQuery.bindValue(":is_night", bus->is_night);
        updateQuery.bindValue(":is_day", bus->is_day);
        updateQuery.bindValue(":is_available", bus->is_available);
        updateQuery.bindValue(":price", bus->price);
        if (!updateQuery.exec()) {
            //// qDebug() <<  Q_FUNC_INFO << "Ошибка обновления автобуса:" << updateQuery.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }
        // Получение ID автобуса
        QSqlQuery busIdQuery;
        busIdQuery.prepare("SELECT id FROM buses WHERE name = :name");
        busIdQuery.bindValue(":name", bus->name);
        if (!busIdQuery.exec() || !busIdQuery.next()) {
            // qDebug() <<  Q_FUNC_INFO << "Ошибка получения ID автобуса:" << busIdQuery.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }
        int bus_id = busIdQuery.value(0).toInt();

        // Вставка новых остановок
        for (size_t i = 0; i < bus->stops.size(); ++i) {
            QSqlQuery stopIdQuery;
            stopIdQuery.prepare("SELECT id FROM stops WHERE name = :name");
            stopIdQuery.bindValue(":name", bus->stops[i]->name);
            if (!stopIdQuery.exec() || !stopIdQuery.next()) {
                // qDebug() <<  Q_FUNC_INFO << "Ошибка получения ID остановки:" << stopIdQuery.lastError().text();
                QSqlDatabase::database().rollback();
                return false;
            }
            int stop_id = stopIdQuery.value(0).toInt();

            if (!AddBusStop(bus_id, stop_id, i /*+ 1*/, bus->stops[i]->name)) {
                // qDebug() <<  Q_FUNC_INFO << "Ошибка добавления остановки:" << QSqlDatabase::database().lastError().text();
                QSqlDatabase::database().rollback();
                return false;
            }
        }
    }
    else {
        // если маршрута не существует, добавляем его как новый
        if (!AddBus(bus)) {
            QSqlDatabase::database().rollback();
            return false;
        }
    }

    // Завершение транзакции
    if (!QSqlDatabase::database().commit()) {
        // qDebug() <<  Q_FUNC_INFO << "Ошибка фиксации транзакции:" << QSqlDatabase::database().lastError().text();
        return false;
    }

    return true;
}
std::optional<Bus> DatabaseManager::FindBus(const QStringView name)
{
    QSqlQuery query = ExecuteSelectQuery(
        QString("SELECT * FROM buses WHERE name = '%1';").arg(name)
    );

    if (query.next()) {
        Bus bus;
        bus.name = query.value("name").toString();
        bus.is_roundtrip = query.value("is_roundtrip").toBool();
        bus.color_index = query.value("color_index").toUInt();
        bus.bus_type = StringToBusType(query.value("bus_type").toString());
        bus.capacity = query.value("capacity").toUInt();
        bus.has_wifi = query.value("has_wifi").toBool();
        bus.has_sockets = query.value("has_sockets").toBool();
        bus.is_night = query.value("is_night").toBool();
        bus.is_day = query.value("is_day").toBool();
        bus.is_available = query.value("is_available").toBool();
        bus.price = query.value("price").toUInt();

        QSqlQuery stops_query = ExecuteSelectQuery(
            QString("SELECT s.name, s.latitude, s.longitude FROM bus_stops bs JOIN stops s ON bs.stop_id = s.id WHERE bs.bus_id = %1;").arg(query.value("id").toInt())
        );

        static std::vector<std::unique_ptr<Stop>> stops_storage;
        while (stops_query.next()) {
            auto stop_ptr = std::make_unique<Stop>();
            stop_ptr->name = stops_query.value("name").toString();
            stop_ptr->coords.lat = stops_query.value("latitude").toDouble();
            stop_ptr->coords.lng = stops_query.value("longitude").toDouble();

            bus.stops.emplace_back(stop_ptr.get());
            stops_storage.emplace_back(std::move(stop_ptr));
        }
        return bus;
    }
    return std::nullopt;
}

bool DatabaseManager::DeleteBus(const Bus* bus) {
    QSqlQuery query;
    query.prepare("DELETE FROM public.buses WHERE name = :name");
    query.bindValue(":name", bus->name);

    if (query.exec()) {
        return true;
    }
    else{
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
    }
    return false;
}

std::optional<svg::Color> DatabaseManager::FindColor(const size_t color_index) {
    QSqlQuery query = ExecuteSelectQuery(
        QString("SELECT unnest(color) AS color_value FROM color_palette WHERE id = %1;").arg(color_index)
    );

    int rgbValues[3] = { 0 };
    size_t index = 0;

    while (query.next() && index < 3) {
        rgbValues[index++] = query.value(0).toInt();
    }

    if (index == 3) {
        return svg::Color(svg::Rgb(rgbValues[0], rgbValues[1], rgbValues[2]));
    }
    return std::nullopt;
}

// Проверка существования связки bus_stop
bool DatabaseManager::BusStopIsExists(int bus_id, const QString& stop_name) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM bus_stops WHERE bus_id = :bus_id AND stop_id = (SELECT id FROM stops WHERE name = ':stop_name'); ");
    query.bindValue(":bus_id", bus_id);
    query.bindValue(":stop_name", stop_name);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    else{
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
    }
    return false;
}

bool DatabaseManager::AddBusStop(int bus_id, const QString& stop_name) {
    if (BusStopIsExists(bus_id, stop_name)) {
        // qDebug() << Q_FUNC_INFO << "Связка bus_stop уже существует:" << bus_id << "->" << stop_name;
        return true;
    }

    QSqlQuery positionQuery;
    positionQuery.prepare("SELECT COALESCE(MAX(stop_position), 0) + 1 FROM bus_stops WHERE bus_id = :bus_id");
    positionQuery.bindValue(":bus_id", bus_id);
    if (!positionQuery.exec() || !positionQuery.next()) {
        // qDebug() << Q_FUNC_INFO << positionQuery.lastError().text();
        return false;
    }
    int stop_position = positionQuery.value(0).toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO bus_stops (bus_id, stop_id, stop_position) "
                  "VALUES (:bus_id, (SELECT id FROM stops WHERE name = :stop_name), :stop_position)");
    query.bindValue(":bus_id", bus_id);
    query.bindValue(":stop_name", stop_name);
    query.bindValue(":stop_position", stop_position);

    if (query.exec()) {
        return true;
    } else {
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
    }
    return false;
}

bool DatabaseManager::AddBusStop(int bus_id, int stop_id, int stop_position, const QString& stop_name)
{
    if (BusStopIsExists(bus_id, stop_name)) {
        return true;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO bus_stops VALUES (:bus_id, :stop_id, :stop_position)");
    query.bindValue(":bus_id", bus_id);
    query.bindValue(":stop_id", stop_id);
    query.bindValue(":stop_position", stop_position);
    if (query.exec()) {
        return true;
    }
    else{
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
    }
    return false;
}

bool DatabaseManager::UpdateRoutingSettings(const double bus_velocity, const int bus_wait_time)
{
    constexpr double epsilon = 1e-9;
    if (bus_velocity > epsilon) {
        if (bus_wait_time > epsilon) {
            QSqlQuery query;
            query.prepare("UPDATE public.routing_settings SET bus_velocity = :bus_velocity, bus_wait_time = :bus_wait_time");
            query.bindValue(":bus_velocity", bus_velocity);
            query.bindValue(":bus_wait_time", bus_wait_time);
            if (query.exec()) {
                return true;
            }
            else{
                // qDebug() << Q_FUNC_INFO << query.lastError().text();
            }
        }
    }
    return false;
}

// Выполнение запроса без возврата данных
void DatabaseManager::ExecuteQuery(const QString& queryStr) {
    QSqlQuery query;
    if (!query.exec(queryStr)) {
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
    }
}

// Выполнение запроса с возвратом данных
QSqlQuery DatabaseManager::ExecuteSelectQuery(const QString& queryStr) {
    QSqlQuery query;
    if (!query.exec(queryStr)) {
        // qDebug() << Q_FUNC_INFO << query.lastError().text();
    }
    return query;
}

const int DatabaseManager::GetRowsCount(QStringView table_name) const
{
   QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM information_schema.columns WHERE table_name = :table_name;");
    query.bindValue(":table_name", table_name.toString());
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return false;
}

