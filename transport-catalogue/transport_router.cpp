#include "transport_router.h"

namespace catalogue {
namespace routing {
graph::Edge<double> RoutingSettings::BuildEdge(const Stop* from, const Stop* to, size_t& vertex_id, double road_distance) {
    double time_for_edge = ComputeTimeForEdge(bus_wait_time_, bus_velocity_, road_distance);

    if (auto it = stops_to_vertex_id_.find(from); it == stops_to_vertex_id_.end()) {
        stops_to_vertex_id_.insert({from, vertex_id});
        vertex_id_to_stops_.insert({vertex_id, from});
        ++vertex_id;
    }
    if (auto it = stops_to_vertex_id_.find(to); it == stops_to_vertex_id_.end()) {
        stops_to_vertex_id_.insert({to, vertex_id});
        vertex_id_to_stops_.insert({vertex_id, to});
        ++vertex_id;
    }

    size_t index_from = stops_to_vertex_id_[from];
    size_t index_to = stops_to_vertex_id_[to];
    graph::Edge<double> edge(index_from, index_to, time_for_edge);
    return edge;
}

void RoutingSettings::AddEdge(const graph::Edge<double>& edge, const Bus* bus_ptr, size_t& edge_id, int span_count) {
    graph_.AddEdge(edge);
    buses_to_edge_id_.insert({edge_id, std::make_pair(bus_ptr, span_count)});
    ++edge_id;
}

void RoutingSettings::BuildGraph(const stat::RequestHandler &rh) {
    std::unordered_set<const Bus*> all_buses = rh.GetBusesPtr();
    std::unordered_set<const Stop*> all_stops_set = rh.GetStopsPtr();
    graph::DirectedWeightedGraph<double> graph(all_stops_set.size());
    
    graph_ = graph;
    
    size_t vertex_id = 0;
    size_t edge_id = 0;

    for (const auto bus_ptr : all_buses) {
        std::vector<const Stop*> all_stops = bus_ptr->GetBusAndStops();
        if (!all_stops.empty()) {
            auto start = all_stops.begin();
            auto finish = all_stops.end();
            if (!bus_ptr->IsRing()) {
                finish = std::next(start, all_stops.size() / 2 + 1);
            }

            for (auto it = start; it != std::prev(finish); ++it) {
                auto first = it;
                auto prev_last = it;
                auto last = std::next(it);

                double road_distance = 0.0;
                double road_distance_reverse = 0.0;

                while (last != finish) {
                    const Stop* from = *first;
                    const Stop *prev = *prev_last;
                    const Stop* to = *last;

                    road_distance += rh.ComputeDistance(prev, to);
                    int span_count = static_cast<int>(std::distance(first, last));

                    const graph::Edge<double> edge = BuildEdge(from, to, vertex_id, road_distance);
                    AddEdge(edge, bus_ptr, edge_id, span_count);

                    if (!bus_ptr->IsRing()) {
                        road_distance_reverse += rh.ComputeDistance(to, prev);
                        const graph::Edge<double> edge = BuildEdge(to, from, vertex_id, road_distance_reverse);
                        AddEdge(edge, bus_ptr, edge_id, span_count);
                    }
                    ++prev_last;
                    ++last;
                }
            }
        }
    }
}

double ComputeTimeForEdge (int bus_wait_time, double bus_velocity, int road_distance) {
    return bus_wait_time * 1.0 + road_distance / (bus_velocity * 1000 / 60 );
}

std::optional<RouteInform> GetRoutingItems(const graph::Router<double>& router, routing::RoutingSettings& rt, const Stop* from_ptr, const Stop* to_ptr) {
    std::optional<RouteInform> result;
    std::vector<RoutingItems> res;

    auto it1 = rt.stops_to_vertex_id_.find(from_ptr);
    auto it2 = rt.stops_to_vertex_id_.find(to_ptr);
    if (it1 == rt.stops_to_vertex_id_.end() || it2 == rt.stops_to_vertex_id_.end()) {
        return result;
    }

    std::optional<typename graph::Router<double>::RouteInfo> route_info = router.BuildRoute(rt.stops_to_vertex_id_.at(from_ptr), rt.stops_to_vertex_id_.at(to_ptr));

    if (route_info) {
        std::vector<graph::EdgeId> edges = route_info->edges;
        double total_time = route_info->weight;

        if (!edges.empty()) {
            for (const auto& edge : edges) {
                const graph::Edge<double> cur_edge = rt.graph_.GetEdge(edge);
                std::string_view stop_name = rt.vertex_id_to_stops_[cur_edge.from]->GetStop();
                std::string_view bus_name = rt.buses_to_edge_id_[edge].first->GetBus();
                int span_count = rt.buses_to_edge_id_[edge].second;
                double time = cur_edge.weight - rt.bus_wait_time_;

                RoutingItems item(stop_name, rt.bus_wait_time_, bus_name, span_count, time);
                res.push_back(item);
            }
        }

        RouteInform route_inform(total_time, res);
        result = route_inform;
    } 

    return result;
}
}// namespace routing
}// namespace catalogue