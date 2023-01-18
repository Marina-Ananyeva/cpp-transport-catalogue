#include "json_reader.h"

namespace catalogue {
namespace reader {
void JSONReader::ParseQuery(std::istream& is, Query &q) {
    json::Dict text_query{json::Load(is).GetRoot().AsDict()};
    for (const auto& [key, val] : text_query) {
        using namespace std::literals;
        if (key == "base_requests"s) {
            for (const auto& v : val.AsArray()) {
                if (v.AsDict().at("type"s) == "Stop"s) {
                    q.text_base_stops_.push_back(v.AsDict());
                }
                if (v.AsDict().at("type"s) == "Bus"s) {
                    q.text_base_buses_.push_back(v.AsDict());
                }
            }
        }

        if (key == "stat_requests"s) {
            for (const auto& v : val.AsArray()) {
                q.text_stat_.push_back(v.AsDict());
            }
        }

        if (key == "render_settings"s) {
            q.text_render_setting_ = val.AsDict();
        }
    }
}

Stop JSONReader::ParseQueryStop(const json::Dict& stops) {
    using namespace std::literals;
    std::string_view stop = stops.at("name"s).AsString();
    double latitude = stops.at("latitude"s).AsDouble();
    double longitude = stops.at("longitude"s).AsDouble();

    return Stop(stop, latitude, longitude);
}

std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> JSONReader::ParseQueryDistance(const stat::RequestHandler& rh, const json::Dict& stops) {
    std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> stops_distance;
    using namespace std::literals;
    std::string_view stop_start = stops.at("name"s).AsString();
    for (const auto& [key, val] : stops.at("road_distances"s).AsDict()) {
            std::string_view stop_finish = key;
            int distance = val.AsInt();
            stops_distance.push_back(std::make_pair(std::make_pair(rh.GetStopPtr(stop_start), rh.GetStopPtr(stop_finish)), distance));
    }

    return stops_distance;
}

Bus JSONReader::ParseQueryBus(const json::Dict& buses) {
    using namespace std::literals;
    std::string_view bus = buses.at("name"s).AsString();
    bool is_ring = buses.at("is_roundtrip"s).AsBool();

    return Bus(bus, is_ring);
}

std::pair<const Bus*, std::vector<const Stop*>> JSONReader::ParseQueryBusRoute(const stat::RequestHandler& rh, const json::Dict& buses) {
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

svg::Color JSONReader::ReadColor(const json::Node& color_setting) {
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

void JSONReader::AddRenderSetting(renderer::RenderSetting&r, const json::Dict&& setting) {
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

json::Dict JSONReader::MakeJsonDocStopsForBus(const int query_id, const stat::StopsForBusStat& r) {
    json::Builder result{};
    using namespace std::literals;
    if (std::get<0>(r.stops_for_bus_) == 0) {
        result.StartDict()
                    .Key("request_id"s).Value(query_id)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
    } else {
        result.StartDict()
                    .Key("request_id"s).Value(query_id)
                    .Key("stop_count"s).Value(std::get<0>(r.stops_for_bus_))
                    .Key("unique_stop_count"s).Value(std::get<1>(r.stops_for_bus_))
                    .Key("route_length"s).Value(static_cast<double>(std::get<2>(r.stops_for_bus_)))
                    .Key("curvature"s).Value(std::get<3>(r.stops_for_bus_))
                    .EndDict()
                    .Build();
    }
    return result.GetNode().AsDict();
}

json::Dict JSONReader::MakeJsonDocBusesForStop(const int query_id, const stat::BusesForStopStat& r) {
    json::Builder result{};
    using namespace std::literals;
    if (r.buses_for_stop_.empty()) {
        result.StartDict()
                    .Key("request_id"s).Value(query_id)
                    .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build();
    } else {
        json::Builder res{};
        res.StartArray();
        if (*r.buses_for_stop_.begin() != "no buses"sv) {
            for (const auto bus : r.buses_for_stop_) {
                std::string bus_str{bus};
                res.Value(bus_str);
            }
        }
        res.EndArray().Build();
        result.StartDict()
                    .Key("request_id"s).Value(query_id)
                    .Key("buses"s).Value(res.GetNode().AsArray())
                    .EndDict()
                    .Build();
    }
    return result.GetNode().AsDict();
}

void FillCatalogue(head::TransportCatalogue& tc, Query& q, renderer::RenderSetting& r, renderer::MapObjects& m, std::istream& is) {
    stat::RequestHandler rh(tc);
    JSONReader reader;
    {
        using namespace std::literals;
        LOG_DURATION("ParseQuery"s);
            reader.ParseQuery(is, q);
    }

    if (!q.text_base_stops_.empty()) {
        {
            using namespace std::literals;
            LOG_DURATION("AddStop"s);
            for (const auto& stop : std::move(q.text_base_stops_)) {
                const Stop new_stop = reader.ParseQueryStop(stop.AsDict());
                tc.AddStop(new_stop);
            }
            tc.AddStopDirectory();
        }
        {
            using namespace std::literals;
            LOG_DURATION("AddDistance"s);
            for (const auto& stop : std::move(q.text_base_stops_)) {
                std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> stops_distance = reader.ParseQueryDistance(rh, stop.AsDict());
                tc.AddDistance(stops_distance);
            }
        }
    }

    if (!q.text_base_buses_.empty()) {
        {
            using namespace std::literals;
            LOG_DURATION("AddBus"s);
            for (const auto& bus : std::move(q.text_base_buses_)) {
                Bus new_bus = reader.ParseQueryBus(bus.AsDict());
                tc.AddBus(new_bus);
            }
            tc.AddBusDirectory();
            
        }
        {
            using namespace std::literals;
            LOG_DURATION("AddRoute"s);
            for (const auto& bus : std::move(q.text_base_buses_)) {
                std::pair<const Bus*, std::vector<const Stop*>> bus_route = reader.ParseQueryBusRoute(rh, bus.AsDict());
                tc.AddRoute(bus_route);
            }
        }
    }

        if (!q.text_render_setting_.empty()) {
        {
            using namespace std::literals;
            LOG_DURATION("AddRenderSetting"s);
            reader.AddRenderSetting(r, std::move(q.text_render_setting_));
        }
        {
            using namespace std::literals;
            LOG_DURATION("RenderMap"s);
            renderer::MapRenderer(rh, r, m);
        }
    }
}

void ExecuteStatRequests(head::TransportCatalogue& tc, reader::Query& q, renderer::MapObjects& m, std::ostream& os) {
    using namespace std::literals;
    LOG_DURATION("GetInfo"s);

    stat::RequestHandler rh(tc);
    JSONReader maker;
    int query_id = 0;
    std::string_view name;

    json::Builder result{};
    result.StartArray();
    if (!q.text_stat_.empty()) {
        for (const auto& query : q.text_stat_) {
            query_id = query.AsDict().at("id"s).AsInt();

            if (query.AsDict().at("type"s) == "Stop"s) {
                name = query.AsDict().at("name"s).AsString();
                result.Value(maker.MakeJsonDocBusesForStop(query_id, stat::GetBusesForStop(rh, name)));
                continue;
            }
            if (query.AsDict().at("type"s) == "Bus"s) {
                name = query.AsDict().at("name"s).AsString();
                result.Value(maker.MakeJsonDocStopsForBus(query_id, stat::GetStopsForBus(rh, name)));
                continue;
            }
            if (query.AsDict().at("type"s) == "Map"s) {
                std::ostringstream output;
                m.map_object_detail_.Render(output);
                std::string map_as_string(output.str());
                result.Value(json::Builder{}.StartDict()
                                                .Key("request_id"s).Value(query_id)
                                                .Key("map"s).Value(map_as_string)
                                                .EndDict()
                                                .Build().AsDict());
            }
        }
    }
    result.EndArray().Build();
    json::Print(json::Document{result.GetNode().AsArray()}, os);
}
}//namespace reader
}//namespace catalogue
