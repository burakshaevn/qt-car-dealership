/*
 * Наполнение транспортного справочника данными из JSON,
 * Обработка запросов к базе и формирование массива ответов в формате JSON
 */

#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include "map_renderer.h"
#include "request_handler.h"
#include "json_builder.h"
#include "transport_router.h"
#include <iostream>
#include <sstream>
#include <QString>

class RequestHandler;

class JsonReader {
public:
    JsonReader() = default;
    JsonReader(std::istream& input)
        : input_(json::Load(input)) 
    {}

    const json::Node& GetBaseRequests() const;

    const json::Node& GetRoutingSettings() const;

    const json::Node& GetRenderSettings() const;

    const json::Node& GetStatRequests() const;

    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;

    void PullCatalogue(TransportCatalogue& catalogue);

    void ProcessStopRequests(const json::Array& arr, TransportCatalogue& catalogue);

    void ProcessBusRequests(const json::Array& arr, TransportCatalogue& catalogue); 
    
    renderer::MapRenderer PullRenderSettings(const json::Dict& request_map) const;
    
    TransportRouter PullRoutingSettings(const json::Node& settings_map, const TransportCatalogue& catalogue) const;

    const json::Node PrintBus(const json::Dict& request_map, RequestHandler& rh) const;

    const json::Node PrintStop(const json::Dict& request_map, RequestHandler& rh) const;

    const json::Node PrintMap(const json::Dict& request_map, RequestHandler& rh) const;
    
    const json::Node PrintRoute(const json::Dict& request_map, RequestHandler& rh) const;

    svg::Color PullColor(const json::Node& color_node) const;

private:
    json::Document input_;  

    std::tuple<QStringView, detail::Coordinates, std::map<QStringView, int>> PullStop(const json::Dict& request_map) const;
    
    void PullStopDistances(TransportCatalogue& catalogue) const;
    
    /*std::tuple<QStringView, std::vector<const Stop*>, bool, size_t, std::array<uint8_t, 3>, BusType, size_t, bool, bool, bool, bool, uint8_t>*/Bus PullBus(const json::Dict& request_map, TransportCatalogue& catalogue) const;
};
