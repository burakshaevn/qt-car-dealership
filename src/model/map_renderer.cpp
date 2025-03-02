/*
* Визуализация карты маршрутов в формате SVG. 
*/

#include "map_renderer.h"

namespace renderer {
    std::vector<svg::Polyline> MapRenderer::GetRouteLines(const std::map<QStringView, Bus*>& buses, const SphereProjector& sp) const {
        std::vector<svg::Polyline> result;
        //size_t color_num = 0;
        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops.empty()) {
                continue;
            }
            std::vector<std::shared_ptr<const Stop>> route_stops{ bus->stops.begin(), bus->stops.end() };
            if (bus->is_roundtrip == false) {
                route_stops.insert(route_stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
            }
            svg::Polyline line;
            for (const auto& stop : route_stops) {
                line.AddPoint(sp(stop->coords));
            }
            line.SetStrokeColor(render_settings_.color_palette[bus->color_index/*color_num*/]);
            line.SetFillColor("none");
            line.SetStrokeWidth(render_settings_.line_width);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            /*if (color_num < (render_settings_.color_palette.size() - 1)) {
                ++color_num;
            }
            else {
                color_num = 0;
            }*/
            result.push_back(line);
        }

        return result;
    }

    std::vector<svg::Text> MapRenderer::GetBusLabel(const std::map<QStringView, Bus*>& buses, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        //size_t color_num = 0;
        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops.empty()) {
                continue;
            }
            svg::Text text;
            svg::Text underlayer;
            text.SetPosition(sp(bus->stops[0]->coords));
            text.SetOffset(render_settings_.bus_label_offset);
            text.SetFontSize(render_settings_.bus_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetFontWeight("bold");
            text.SetData(bus->name.toStdString());
            text.SetFillColor(render_settings_.color_palette[bus->color_index/*color_num*/]);
            
            /*if (color_num < (render_settings_.color_palette.size() - 1)) {
                ++color_num;
            }
            else {
                color_num = 0;
            }*/

            underlayer.SetPosition(sp(bus->stops[0]->coords));
            underlayer.SetOffset(render_settings_.bus_label_offset);
            underlayer.SetFontSize(render_settings_.bus_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetFontWeight("bold");
            underlayer.SetData(bus->name.toStdString());
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);

            if (bus->is_roundtrip == false && bus->stops[0] != bus->stops[bus->stops.size() - 1]) {
                svg::Text text2{ text };
                svg::Text underlayer2{ underlayer };
                text2.SetPosition(sp(bus->stops[bus->stops.size() - 1]->coords));
                underlayer2.SetPosition(sp(bus->stops[bus->stops.size() - 1]->coords));

                result.push_back(underlayer2);
                result.push_back(text2);
            }
        }
        return result;
    }

    std::vector<svg::Circle> MapRenderer::GetStopsSymbols(const std::map<QStringView, const Stop*>& stops, const SphereProjector& sp) const {
        std::vector<svg::Circle> result;
        for (const auto& [stop_name, stop] : stops) {
            svg::Circle symbol;
            symbol.SetCenter(sp(stop->coords));
            symbol.SetRadius(render_settings_.stop_radius);
            symbol.SetFillColor("white");

            result.push_back(symbol);
        }

        return result;
    }

    std::vector<svg::Text> MapRenderer::GetStopsLabels(const std::map<QStringView, const Stop*>& stops, const SphereProjector& sp) const {
        std::vector<svg::Text> result;
        svg::Text text;
        svg::Text underlayer;
        for (const auto& [stop_name, stop] : stops) {
            text.SetPosition(sp(stop->coords));
            text.SetOffset(render_settings_.stop_label_offset);
            text.SetFontSize(render_settings_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name.toStdString());
            text.SetFillColor("black");

            underlayer.SetPosition(sp(stop->coords));
            underlayer.SetOffset(render_settings_.stop_label_offset);
            underlayer.SetFontSize(render_settings_.stop_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetData(stop->name.toStdString());
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            result.push_back(underlayer);
            result.push_back(text);
        }

        return result;
    }

    svg::Document MapRenderer::GetSVG(const std::map<QStringView, Bus*>& buses) const {
        svg::Document result;
        std::vector<detail::Coordinates> route_stops_coord;
        std::map<QStringView, const Stop*> all_stops;

        // Преобразуем std::map<QString, Bus> в std::map<QStringView, const Bus*>
        /*std::map<QStringView, const Bus*> buses_view;
        for (const auto& [bus_number, bus] : buses) {
            buses_view[bus_number] = &bus;
        }*/

        for (const auto& [bus_number, bus] : buses) {
            if (bus->stops.empty()) {
                continue;
            }
            for (const auto& stop : bus->stops) {
                route_stops_coord.push_back(stop->coords);
                all_stops[stop->name] = stop.get();
            }
        }

        SphereProjector sp(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);

        // Вызываем методы с преобразованной картой
        DrawRouteLines(buses, sp, result);
        DrawBusLabel(buses, sp, result);
        DrawStopsSymbols(all_stops, sp, result);
        DrawStopsLabels(all_stops, sp, result);

        return result;
    }

    void MapRenderer::DrawRouteLines(const std::map<QStringView, Bus*>& buses, SphereProjector& sp, svg::Document& result) const {
        for (const auto& line : GetRouteLines(buses, sp)) {
            result.Add(line);
        }
    }
    void MapRenderer::DrawBusLabel(const std::map<QStringView, Bus*>& buses, SphereProjector& sp, svg::Document& result) const {
        for (const auto& text : GetBusLabel(buses, sp)) {
            result.Add(text);
        }
    }
    void MapRenderer::DrawStopsSymbols(std::map<QStringView, const Stop*>& all_stops, SphereProjector& sp, svg::Document& result) const {
        for (const auto& circle : GetStopsSymbols(all_stops, sp)) {
            result.Add(circle);
        }
    }
    void MapRenderer::DrawStopsLabels(std::map<QStringView, const Stop*>& all_stops, SphereProjector& sp, svg::Document& result) const {
        for (const auto& text : GetStopsLabels(all_stops, sp)) {
            result.Add(text);
        }
    }
} // namespace renderer
