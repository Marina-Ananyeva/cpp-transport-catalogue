#include "json_reader.h"

namespace catalogue {
namespace reader {
void ParseQuery(std::istream& is, Query &q) {
    json::Dict text_query{json::Load(is).GetRoot().AsMap()};
    for (const auto& [key, val] : text_query) {
        using namespace std::literals;
        if (key == "base_requests"s) {
            for (const auto& v : val.AsArray()) {
                if (v.AsMap().at("type"s) == "Stop"s) {
                    q.text_base_stops_.push_back(v.AsMap());
                }
                if (v.AsMap().at("type"s) == "Bus"s) {
                    q.text_base_buses_.push_back(v.AsMap());
                }
            }
        }

        if (key == "stat_requests"s) {
            for (const auto& v : val.AsArray()) {
                q.text_stat_.push_back(v.AsMap());
            }
        }

        if (key == "render_settings"s) {
            q.text_render_setting_ = val.AsMap();
        }
    }
}

Stop ParseQueryStop(const json::Dict& stops) {
    using namespace std::literals;
    std::string_view stop = stops.at("name"s).AsString();
    double latitude = stops.at("latitude"s).AsDouble();
    double longitude = stops.at("longitude"s).AsDouble();

    return Stop(stop, latitude, longitude);
}

std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> ParseQueryDistance(const stat::RequestHandler& rh, const json::Dict& stops) {
    std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> stops_distance;
    using namespace std::literals;
    std::string_view stop_start = stops.at("name"s).AsString();
    for (const auto& [key, val] : stops.at("road_distances"s).AsMap()) {
            std::string_view stop_finish = key;
            int distance = val.AsInt();
            stops_distance.push_back(std::make_pair(std::make_pair(rh.GetStopPtr(stop_start), rh.GetStopPtr(stop_finish)), distance));
    }

    return stops_distance;
}

Bus ParseQueryBus(const json::Dict& buses) {
    using namespace std::literals;
    std::string_view bus = buses.at("name"s).AsString();
    bool is_ring = buses.at("is_roundtrip"s).AsBool();

    return Bus(bus, is_ring);
}

std::pair<const Bus*, std::vector<const Stop*>> ParseQueryBusRoute(const stat::RequestHandler& rh, const json::Dict& buses) {
    std::string_view stop_route;
    std::vector<const Stop*> bus_and_stops;

    using namespace std::literals;

    std::string_view bus = buses.at("name"s).AsString();
    for (const auto& bus : buses.at("stops"s).AsArray()) {
            stop_route = bus.AsString();
            bus_and_stops.push_back(rh.GetStopPtr(stop_route));
    }

    return std::make_pair(rh.GetBusPtr(bus), bus_and_stops);
}

svg::Color ReadColor(const json::Node& color_setting) {
    bool type = color_setting.IsString();

    if (type) {
        return svg::Color{color_setting.AsString()};
    } 

    int red = color_setting.AsArray()[0].AsInt();
    int green = color_setting.AsArray()[1].AsInt();
    int blue = color_setting.AsArray()[2].AsInt();

    size_t count = color_setting.AsArray().size();
    if (count == 3) {
        return svg::Color{svg::Rgb{red, green, blue}};
    }

    double opacity = color_setting.AsArray()[3].AsDouble();
    return svg::Color{svg::Rgba(red, green, blue, opacity)};
}

void AddRenderSetting(renderer::RenderSetting&r, const json::Dict&& setting) {
    using namespace std::literals;
    r.width_ = setting.at("width"s).AsDouble();
    r.height_ = setting.at("height"s).AsDouble();
    r.padding_ = setting.at("padding"s).AsDouble();
    r.line_width_ = setting.at("line_width"s).AsDouble();
    r.stop_radius_ = setting.at("stop_radius"s).AsDouble();
    r.bus_label_font_size_ = setting.at("bus_label_font_size"s).AsInt();
    r.bus_label_offset_.x = setting.at("bus_label_offset"s).AsArray()[0].AsDouble();
    r.bus_label_offset_.y = setting.at("bus_label_offset"s).AsArray()[1].AsDouble();
    r.stop_label_font_size_ = setting.at("stop_label_font_size"s).AsInt();
    r.stop_label_offset_.x = setting.at("stop_label_offset"s).AsArray()[0].AsDouble();
    r.stop_label_offset_.y = setting.at("stop_label_offset"s).AsArray()[1].AsDouble();
    r.underlayer_width_ = setting.at("underlayer_width"s).AsDouble();

    json::Node color_setting{setting.at("underlayer_color"s)};
    r.underlayer_color_ = ReadColor(color_setting);

    for (const auto& color : setting.at("color_palette"s).AsArray()) {
        r.color_palette_.push_back(ReadColor(color));
    }
}

void FillCatalogue(head::TransportCatalogue& tc, Query& q, renderer::RenderSetting& r, renderer::MapObjects& m, std::istream& is) {
    stat::RequestHandler rh(tc);
    {
        using namespace std::literals;
        LOG_DURATION("ParseQuery"s);
            ParseQuery(is, q);
    }

    if (!q.text_base_stops_.empty()) {
        {
            using namespace std::literals;
            LOG_DURATION("AddStop"s);
            for (const auto& stop : std::move(q.text_base_stops_)) {
                const Stop new_stop = ParseQueryStop(stop.AsMap());
                tc.AddStop(new_stop);
            }
            tc.AddStopDirectory();
        }
        {
            using namespace std::literals;
            LOG_DURATION("AddDistance"s);
            for (const auto& stop : std::move(q.text_base_stops_)) {
                std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> stops_distance = ParseQueryDistance(rh, stop.AsMap());
                tc.AddDistance(stops_distance);
            }
        }
    }

    if (!q.text_base_buses_.empty()) {
        {
            using namespace std::literals;
            LOG_DURATION("AddBus"s);
            for (const auto& bus : std::move(q.text_base_buses_)) {
                Bus new_bus = ParseQueryBus(bus.AsMap());
                tc.AddBus(new_bus);
            }
            tc.AddBusDirectory();
            
        }
        {
            using namespace std::literals;
            LOG_DURATION("AddRoute"s);
            for (const auto& bus : std::move(q.text_base_buses_)) {
                std::pair<const Bus*, std::vector<const Stop*>> bus_route = ParseQueryBusRoute(rh, bus.AsMap());
                tc.AddRoute(bus_route);
            }
        }
    }

        if (!q.text_render_setting_.empty()) {
        {
            using namespace std::literals;
            LOG_DURATION("AddRenderSetting"s);
            AddRenderSetting(r, std::move(q.text_render_setting_));
        }
        {
            using namespace std::literals;
            LOG_DURATION("RenderMap"s);
            renderer::MapRenderer(rh, r, m);
        }
    }
}

json::Dict MakeJsonDocumentStopsForBus(const int query_id, const stat::StopsForBusStat& r) {
    json::Dict result;
    using namespace std::literals;
    if (std::get<0>(r.stops_for_bus_) == 0) {
        result.insert({
            {"request_id"s, query_id},
            {"error_message"s, "not found"s},
        });
    } else {
        result.insert({
            {"request_id"s, query_id},
            {"stop_count"s, std::get<0>(r.stops_for_bus_)},
            {"unique_stop_count"s, std::get<1>(r.stops_for_bus_)},
            {"route_length"s, static_cast<double>(std::get<2>(r.stops_for_bus_))},
            {"curvature"s, std::get<3>(r.stops_for_bus_)},
        });
    }
    return result;
}

json::Dict MakeJsonDocumentBusesForStop(const int query_id, const stat::BusesForStopStat& r) {
    json::Dict result;
    using namespace std::literals;
    if (r.buses_for_stop_.empty()) {
        result.insert({
            {"request_id"s, query_id},
            {"error_message"s, "not found"s},
        });
    } else {
        json::Array res;
        if (*r.buses_for_stop_.begin() != "no buses"sv) {
            for (const auto bus : r.buses_for_stop_) {
                std::string bus_str{bus};
                res.push_back(bus_str);
            }
        }

        result.insert({
            {"request_id"s, query_id},
            {"buses"s, res},
        });
    }
    return result;
}

void ExecuteStatRequests(head::TransportCatalogue& tc, reader::Query& q, renderer::MapObjects& m, std::ostream& os) {
    using namespace std::literals;
    LOG_DURATION("GetInfo"s);
    stat::RequestHandler rh(tc);
    json::Array result;
    int query_id = 0;
    std::string_view name;
    if (!q.text_stat_.empty()) {
        for (const auto& query : q.text_stat_) {
            using namespace std::literals;
            query_id = query.AsMap().at("id"s).AsInt();

            if (query.AsMap().at("type"s) == "Stop"s) {
                name = query.AsMap().at("name"s).AsString();
                result.push_back(MakeJsonDocumentBusesForStop(query_id, stat::GetBusesForStop(rh, name)));
                continue;
            }
            if (query.AsMap().at("type"s) == "Bus"s) {
                name = query.AsMap().at("name"s).AsString();
                result.push_back(MakeJsonDocumentStopsForBus(query_id, stat::GetStopsForBus(rh, name)));
                continue;
            }
            if (query.AsMap().at("type"s) == "Map"s) {
                std::ostringstream output;
                m.map_object_detail_.Render(output);
                std::string map_as_string(output.str());
                result.push_back(json::Dict{
                    {"request_id"s, query_id},
                    {"map"s, map_as_string},
                    });
            }
        }
    }
    json::Print(json::Document{result}, os);
}
}//namespace reader
}//namespace catalogue
