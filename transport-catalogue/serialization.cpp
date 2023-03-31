#include "serialization.h"

namespace catalogue {
namespace serialize {
void SerializeStops(const TransportCatalogue &tc, TransportCatalogue_s &object, StopsAndBusesDictionary &dict) {
    int stop_number = 0;
    for (const Stop &stop : tc.stops_) {
        Stop_s stop_s;
        *stop_s.mutable_name() = std::string(stop.GetStop());
        stop_s.set_latitude(stop.GetGeo().first);
        stop_s.set_longitude(stop.GetGeo().second);
        *object.add_stops() = stop_s;
        dict.stops_dict_s.insert({stop.GetStop(), stop_number});
        ++stop_number;
    }
}

void SerializeDistance(const TransportCatalogue &tc, TransportCatalogue_s &object, StopsAndBusesDictionary &dict) {
    for (const auto d : tc.stops_distance_) {
        Distance_s distance_info;
        uint32_t stop_number1 = dict.stops_dict_s[d.first.first->GetStop()];
        distance_info.set_start_stop(stop_number1);
        uint32_t stop_number2 = dict.stops_dict_s[d.first.second->GetStop()];
        distance_info.set_finish_stop(stop_number2);
        uint32_t distance = d.second;
        distance_info.set_distance(distance);
        *object.add_distance_info() = distance_info;
    }
}

void SerializeBusesAndRoute(const TransportCatalogue &tc, TransportCatalogue_s &object, StopsAndBusesDictionary &dict) {
    int bus_number = 0;
    for (const Bus &bus : tc.buses_) {
        Bus_s bus_s;
        *bus_s.mutable_name() = std::string(bus.GetBus());
        bus_s.set_is_ring(bus.IsRing());
        *object.add_buses() = bus_s;

        Route_s route_info;
        route_info.set_bus_name(bus_number);
        std::vector<const Stop *> stops = bus.GetBusAndStops();
        int stops_count = static_cast<int>(stops.size());
        if (!bus.IsRing()) {
            stops_count = (stops_count - 1) / 2 + 1;
        }
        for (int i = 0; i < stops_count; ++i) {
            const Stop *stop_ptr = stops[i];
            uint32_t stop_number1 = dict.stops_dict_s[stop_ptr->GetStop()];
            route_info.add_stop_names(stop_number1);
        }
        *object.add_route_info() = route_info;
        dict.buses_dict_s.insert({bus.GetBus(), bus_number});
        ++bus_number;
    }
}

Svg::Point SerializePointSvg(const svg::Point &p) {
    Svg::Point p_s;
    p_s.set_x(p.x);
    p_s.set_y(p.y);
    return p_s;
}

Svg::Color SerializeColorSvg(const svg::Color &c) {
    Svg::Color color_s;
    if (!std::holds_alternative<std::monostate>(c)) {
        if (std::holds_alternative<std::string>(c)) {
            *color_s.mutable_color() = std::get<std::string>(c);
        } else if (std::holds_alternative<svg::Rgb>(c)) {
            Svg::Rgb rgb_s;
            svg::Rgb rgb = std::get<svg::Rgb>(c);
            rgb_s.set_red(rgb.red);
            rgb_s.set_green(rgb.green);
            rgb_s.set_blue(rgb.blue);
            *color_s.mutable_rgb() = rgb_s;
        } else if (std::holds_alternative<svg::Rgba>(c)) {
            Svg::Rgba rgba_s;
            svg::Rgba rgba = std::get<svg::Rgba>(c);
            rgba_s.set_red(rgba.red);
            rgba_s.set_green(rgba.green);
            rgba_s.set_blue(rgba.blue);
            rgba_s.set_opacity(rgba.opacity);
            *color_s.mutable_rgba() = rgba_s;
        }
    }
    return color_s;
}

void SerializeRendererSetting(const TransportCatalogue &tc, const renderer::RenderSettings &r, Map_renderer::MapRenderer &r_s) {
    r_s.set_width(r.width_);
    r_s.set_height(r.height_);
    r_s.set_padding(r.padding_);
    r_s.set_line_width(r.line_width_);
    r_s.set_stop_radius(r.stop_radius_);
    r_s.set_bus_label_font_size(r.bus_label_font_size_);
    r_s.set_stop_label_font_size(r.stop_label_font_size_);
    r_s.set_underlayer_width(r.underlayer_width_);

    *r_s.mutable_bus_label_offset() = SerializePointSvg(r.bus_label_offset_);
    *r_s.mutable_stop_label_offset() = SerializePointSvg(r.stop_label_offset_);
        
    *r_s.mutable_underlayer_color() = SerializeColorSvg(r.underlayer_color_);

    for (const svg::Color c : r.color_palette_) {
        *r_s.add_color_palette() = SerializeColorSvg(c);
    }
}

void SerializeRouterSetting(const routing::RoutingSettings& rt, Router::RouterSetting& rt_s) {
    rt_s.set_bus_velocity(rt.bus_velocity_);
    rt_s.set_bus_wait_time(rt.bus_wait_time_);
}

void SerializeRouter(const routing::RoutingSettings& rt, Router::RouterSetting& rt_s, const StopsAndBusesDictionary &dict) {
    for (const auto& p : rt.stops_to_vertex_id_) {
        int stop = dict.stops_dict_s.at(p.first->GetStop());
        int vertex = p.second;
        Router::StopVertex stop_vertex;
        stop_vertex.set_stop(stop);
        stop_vertex.set_vertex(vertex);
        *rt_s.add_stop_vertex() = stop_vertex;
    }
    for (const auto& p : rt.buses_to_edge_id_) {
        int edge = p.first;
        int bus = dict.buses_dict_s.at(p.second.first->GetBus());
        int span = p.second.second;
        Router::BusEdge bus_edge;
        bus_edge.set_edge(edge);
        bus_edge.set_bus(bus);
        bus_edge.set_span(span);
        *rt_s.add_bus_edge() = bus_edge;
    }
}

void SerializeGraph(const routing::RoutingSettings& rt, Graph::Graph& graph) {
    size_t edges_count = rt.graph_.GetEdgeCount();
    int vertex_count = static_cast<int>(rt.graph_.GetVertexCount());
    graph.set_vertex_count(vertex_count);
    for (size_t i = 0; i < edges_count; ++i) {
        Graph::Edge edge;
        uint32_t from = static_cast<uint32_t>(rt.graph_.GetEdge(i).from);
        uint32_t to = static_cast<uint32_t>(rt.graph_.GetEdge(i).to);
        edge.set_from(from);
        edge.set_to(to);
        edge.set_weight(rt.graph_.GetEdge(i).weight);
        *graph.add_edges() = edge;
    }
}

void SerializeTransportCatalogue(const TransportCatalogue &tc, const reader::Query &q, const renderer::RenderSettings &r, const renderer::MapObjects &m, const routing::RoutingSettings &rt, std::ofstream &out_file) {
    TransportCatalogue_s object;
    StopsAndBusesDictionary dict;

    if (!q.text_base_stops_.empty()) {
        {
            using namespace std::literals;
            //LOG_DURATION("SerializeStops"s);
            SerializeStops(tc, object, dict);
        }
        {
            using namespace std::literals;
            //LOG_DURATION("SerializeDistance"s);
            SerializeDistance(tc, object, dict);
        }
    }

    if (!q.text_base_buses_.empty()) {
        using namespace std::literals;
        //LOG_DURATION("SerializeBusesAndRoute"s);
        SerializeBusesAndRoute(tc, object, dict);
    }

    if (!q.text_render_settings_.empty()) {
        using namespace std::literals;
        //LOG_DURATION("SerializeRenderSetting"s);
        Map_renderer::MapRenderer r_s;
        SerializeRendererSetting(tc, r, r_s);
        *object.mutable_map_renderer() = r_s;
    }

    if (!q.text_routing_settings_.empty()) {
        Router::RouterSetting rt_s;
        {
            using namespace std::literals;
            // LOG_DURATION("SerializeRoutingSettings"s);
            SerializeRouterSetting(rt, rt_s);
        }
        {
            using namespace std::literals;
            // LOG_DURATION("SerializeRouter"s);
            SerializeRouter(rt, rt_s, dict);
        }
        {
            using namespace std::literals;
            // LOG_DURATION("SerializeGraph"s);
            Graph::Graph graph;
            SerializeGraph(rt, graph);
            *rt_s.mutable_graph() = graph;
        }
        *object.mutable_router_setting() = rt_s;
    }

    object.SerializeToOstream(&out_file);
}

void FillStopsAfterDeserialize(const TransportCatalogue_s &tc_s, TransportCatalogue &tc, StopsAndBusesDictionary &dict) {
    int stops_count = tc_s.stops_size();
    using namespace std::literals;
    // LOG_DURATION("FillStops"s);
    for (int i = 0; i < stops_count; ++i) {
        tc.AddStop(Stop(tc_s.stops(i).name(), tc_s.stops(i).latitude(), tc_s.stops(i).longitude()));
        dict.stops_dict_d.insert({i, tc_s.stops(i).name()});
    }

    tc.AddStopDirectory();
}

void FillBusesAfterDeserialize(const TransportCatalogue_s &tc_s, TransportCatalogue &tc, StopsAndBusesDictionary &dict) {
    int buses_count = tc_s.buses_size();
    using namespace std::literals;
    // LOG_DURATION("FillBuses"s);
    for (int i = 0; i < buses_count; ++i) {
        tc.AddBus(Bus(tc_s.buses(i).name(), tc_s.buses(i).is_ring()));
        dict.buses_dict_d.insert({i, tc_s.buses(i).name()});
    }

    tc.AddBusDirectory();
}

void FillDistanceAfterDeserialize(const TransportCatalogue_s &tc_s, TransportCatalogue &tc, const stat::RequestHandler &rh, StopsAndBusesDictionary &dict) {
    int distance_count = tc_s.distance_info_size();

    using namespace std::literals;
    // LOG_DURATION("FillDistance"s);
    std::vector<std::pair<std::pair<const Stop *, const Stop *>, int>> stops_distance(distance_count);
    for (int i = 0; i < distance_count; ++i) {
        const Distance_s distance_s = tc_s.distance_info(i);

        int start_stop_number = distance_s.start_stop();
        std::string_view start_stop_name = dict.stops_dict_d[start_stop_number];
        const Stop *start_stop_ptr = rh.FindStop(start_stop_name);

        int finish_stop_number = distance_s.finish_stop();
        std::string_view finish_stop_name = dict.stops_dict_d[finish_stop_number];
        const Stop *finish_stop_ptr = rh.FindStop(finish_stop_name);

        int distance = distance_s.distance();
        stops_distance[i] = std::make_pair(std::make_pair(start_stop_ptr, finish_stop_ptr), distance);
    }

    tc.AddDistance(stops_distance);
}

void FillRouteAfterDeserialize(const TransportCatalogue_s &tc_s, TransportCatalogue &tc, const stat::RequestHandler &rh, StopsAndBusesDictionary &dict) {
    int route_count = tc_s.route_info_size();

    using namespace std::literals;
    // LOG_DURATION("FillRoute"s);
    for (int i = 0; i < route_count; ++i) {
        Route_s route_s(tc_s.route_info(i));
        int bus_number = route_s.bus_name();
        std::string_view bus_name = dict.buses_dict_d[bus_number];
        const Bus *bus_ptr = rh.FindBus(bus_name);

        int stops_count = route_s.stop_names_size();
        std::vector<const Stop *> stops(stops_count);
        for (int j = 0; j < stops_count; ++j) {
            int stop_number = route_s.stop_names(j);
                std::string_view stop_name = dict.stops_dict_d[stop_number];
                const Stop *stop_ptr = rh.FindStop(stop_name);
                stops[j] = stop_ptr;
        }
            std::pair<const Bus *, std::vector<const Stop *>> bus_route = std::make_pair(bus_ptr, stops);
            tc.AddRoute(bus_route);
    }
}

svg::Color FillColorSvg(const Svg::Color& color_s) {
    svg::Color color;
    if (color_s.has_rgb()) {
        color = svg::Rgb(static_cast<uint8_t>(color_s.rgb().red()), static_cast<uint8_t>(color_s.rgb().green()), static_cast<uint8_t>(color_s.rgb().blue()));
    } else if (color_s.has_rgba()) {
        color = svg::Rgba(static_cast<uint8_t>(color_s.rgba().red()), static_cast<uint8_t>(color_s.rgba().green()), static_cast<uint8_t>(color_s.rgba().blue()), color_s.rgba().opacity());
    } else if (!color_s.color().empty()) {
        if (color_s.color() == "none") {
            color = svg::NoneColor;
        } else {
            color = color_s.color();
        }
    } else {
        color = std::monostate{};
    }
    return color;
}

void FillRenderSetting(const Map_renderer::MapRenderer &r_s, renderer::RenderSettings &r) {
    using namespace std::literals;
    // LOG_DURATION("FillRenderSettings"s);
    r.width_ = r_s.width();
    r.height_ = r_s.height();
    r.padding_ = r_s.padding();
    r.line_width_ = r_s.line_width();
    r.stop_radius_ = r_s.stop_radius();
    r.bus_label_font_size_ = r_s.bus_label_font_size();
    r.stop_label_font_size_ = r_s.stop_label_font_size();
    r.underlayer_width_ = r_s.underlayer_width();

    r.bus_label_offset_ = svg::Point(r_s.bus_label_offset().x(), r_s.bus_label_offset().y());
    r.stop_label_offset_ = svg::Point(r_s.stop_label_offset().x(), r_s.stop_label_offset().y());

    if (r_s.has_underlayer_color()) {
        r.underlayer_color_ = FillColorSvg(r_s.underlayer_color());
    } else {
        r.underlayer_color_ = nullptr;
    }

    int palette_size = r_s.color_palette_size();
    for (int i = 0; i < palette_size; ++i) {
        r.color_palette_.push_back(FillColorSvg(r_s.color_palette(i)));
    }
}

void FillMapObject(const stat::RequestHandler &rh, renderer::RenderSettings &r, renderer::MapObjects &m) {
    using namespace std::literals;
    // LOG_DURATION("RenderMap"s);
    catalogue::renderer::MapRenderer(rh, r, m);
}

void FillRouterSetting(const Router::RouterSetting& rt_s, routing::RoutingSettings& rt) {
    using namespace std::literals;
    // LOG_DURATION("FillRouterSettings"s);
    rt.bus_velocity_ = rt_s.bus_velocity();
    rt.bus_wait_time_ = rt_s.bus_wait_time();
}

void FillRouter(const Router::RouterSetting& rt_s, routing::RoutingSettings& rt, const stat::RequestHandler &rh, const StopsAndBusesDictionary &dict) {
    using namespace std::literals;
    // LOG_DURATION("FillRouter"s);
    int vertex_count = rt_s.stop_vertex_size();
    for (int i = 0; i < vertex_count; ++i) {
        Router::StopVertex stop_vertex = rt_s.stop_vertex(i);
        int vertex = stop_vertex.vertex();
        int stop = stop_vertex.stop();
        const Stop *stop_ptr = rh.FindStop(dict.stops_dict_d.at(stop));
        rt.stops_to_vertex_id_.insert({stop_ptr, vertex});
        rt.vertex_id_to_stops_.insert({vertex, stop_ptr});
    }

    int edge_count = rt_s.bus_edge_size();
    for (int i = 0; i < edge_count; ++i) {
        Router::BusEdge bus_edge = rt_s.bus_edge(i);
        int edge = bus_edge.edge();
        int bus = bus_edge.bus();
        const Bus *bus_ptr = rh.FindBus(dict.buses_dict_d.at(bus));
        int span = bus_edge.span();
        rt.buses_to_edge_id_.insert({edge, std::make_pair(bus_ptr, span)});
    }
}

void FillGraph(const Graph::Graph& graph_s, routing::RoutingSettings& rt) {
    using namespace std::literals;
    // LOG_DURATION("FillGraph"s);
    size_t vertex_count = static_cast<size_t>(graph_s.vertex_count());
    graph::DirectedWeightedGraph<double> graph(vertex_count);
    size_t edge_count = static_cast<size_t>(graph_s.edges_size());
    for (size_t i = 0; i < edge_count; ++i) {
        size_t from = static_cast<size_t>(graph_s.edges(i).from());
        size_t to = static_cast<size_t>(graph_s.edges(i).to());
        double weight = graph_s.edges(i).weight();
        graph::Edge edge(from, to, weight);
        graph.AddEdge(edge);
    }

    rt.graph_ = graph;
}

void FillCatalogueAfterDeserialize(const TransportCatalogue_s &tc_s, TransportCatalogue &tc, renderer::RenderSettings &r, renderer::MapObjects &m, routing::RoutingSettings& rt) {
    stat::RequestHandler rh(tc);
    StopsAndBusesDictionary dict;

    FillStopsAfterDeserialize(tc_s, tc, dict);
    FillBusesAfterDeserialize(tc_s, tc, dict);

    FillDistanceAfterDeserialize(tc_s, tc, rh, dict);
    FillRouteAfterDeserialize(tc_s, tc, rh, dict);

    if (tc_s.has_map_renderer()) {
        FillRenderSetting(tc_s.map_renderer(), r);
        FillMapObject(rh, r, m);
    }

    if (tc_s.has_router_setting()) {
        FillRouterSetting(tc_s.router_setting(), rt);
        FillRouter(tc_s.router_setting(), rt, rh, dict);
        FillGraph(tc_s.router_setting().graph(), rt);
    }
}

void DeserializeTransportCatalogue(std::ifstream &input_file, head::TransportCatalogue &tc, renderer::RenderSettings &r, renderer::MapObjects &m, routing::RoutingSettings& rt) {
    transport_catalogue::Catalogue object;
    object.ParseFromIstream(&input_file);
    FillCatalogueAfterDeserialize(object, tc, r, m, rt);
}
} // namespace serialize
} // namespace catalogue
