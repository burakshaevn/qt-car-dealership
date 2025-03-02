#pragma once

#ifndef DOMAIN_H
#define DOMAIN_H

#include <QString>
#include <vector> 
#include "geo.h"
#include <array>
#include <QDebug>

struct Stop { 
    Stop() = default;
    template<typename T, typename U>
    Stop(const T&& name_, const U&& coords_)
        : name(std::forward<T>(name_))
        , coords(std::forward<U>(coords_))
    {}
    Stop(const QString& name_, const detail::Coordinates& coords_)
        : name(name_)
        , coords(coords_)
    {}
    QString name;
    detail::Coordinates coords; 
};

enum class BusType {
    undefined,
    autobus,
    electrobus,
    trolleybus 
};

QString BusTypeToString(BusType bus_type);

BusType StringToBusType(const QStringView bus_type);

std::vector<QString> SplitIntoWords(const QString& text);

struct Bus {
    Bus() = default;  
    Bus(const Bus&) = default;
    Bus& operator=(const Bus&) = default;
    Bus(Bus&&) = default;
    Bus& operator=(Bus&&) = default; 

    QString name;
    std::vector<std::shared_ptr<const Stop>> stops;
    bool is_roundtrip;
    size_t color_index;

    std::array<uint8_t, 3> rgb;
    BusType bus_type;
    int capacity;
    bool has_wifi;
    bool has_sockets;
    bool is_night;
    bool is_day;

    bool is_available;
    int price;
};

struct BusInfo {
    QString name;
    size_t count_stops;
    size_t unique_count_stops;
    double len;
    double curvature;

    std::array<uint8_t, 3> rgb;
    BusType bus_type;
    int capacity;
    bool has_wifi;
    bool has_sockets;
    bool is_night;
    bool is_day;

    bool is_available;
    int price;
};

#endif // DOMAIN_H
