#pragma once

#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <optional>
#include <memory>
#include <QString>

class RequestHandler {
public:
    using Graph = graph::DirectedWeightedGraph<double>;

    explicit RequestHandler(const TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const TransportRouter& transport_router)
        : catalogue_(catalogue)
        , renderer_(renderer)
        , transport_router_(transport_router)
    {}

    std::optional<BusInfo> GetBusStat(const QStringView bus_number) const;

    svg::Document RenderMap([[maybe_unused]] const QStringView bus_name) const;

    const TransportCatalogue& GetCatalogue() const;

    std::pair<std::optional<graph::Router<double>::RouteInfo>, const TransportRouter::Graph&> GetOptimalRoute(const QStringView from, const QStringView to) const;

private:
    const TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const TransportRouter& transport_router_;
};
