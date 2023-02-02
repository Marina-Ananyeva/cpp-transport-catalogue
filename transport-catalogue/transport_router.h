#pragma once

#include "domain.h"
#include "graph.h"
#include "request_handler.h"
#include "router.h"

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace catalogue {
namespace routing {
using Stop = domain::Stop;
using Bus = domain::Bus;

class RoutingSettings {
public:
    graph::Edge<double> BuildEdge(const Stop* from, const Stop* to, size_t& vertex_id, double road_distance);

    void AddEdge(const graph::Edge<double> &edge, const Bus *ptr, size_t &edge_id, int span_count);

    void BuildGraph(const stat::RequestHandler &rh);

    int bus_wait_time_ = 0;                                                         //время ожидания автобуса
    double bus_velocity_ = 0.0;                                                     //скорость автобуса в км/ч
    graph::DirectedWeightedGraph<double> graph_;                                    //граф
    std::unordered_map<const Stop*, size_t> stops_to_vertex_id_;                    //словарь указатель на остановку - номер вершины графа
    std::unordered_map<size_t, const Stop*> vertex_id_to_stops_;                    //словарь номер вершины графа - указатель на остановку
    std::unordered_map<size_t, std::pair<const Bus*, int>> buses_to_edge_id_;       //словарь номер ребра графа - указатель на маршрут и кол-во остановок на ребре
};

class RoutingItems {
public:
    explicit RoutingItems(std::string_view stop_name, int time_wait, std::string_view bus_name, int span_count, double time)
    : stop_name_(stop_name)
    , time_wait_(time_wait)
    , bus_name_(bus_name)
    , span_count_(span_count)
    , time_(time) {
    }

    std::string_view stop_name_{};
    int time_wait_ = 0;
    std::string_view bus_name_{};
    int span_count_ = 0;
    double time_ = 0.0;
};

struct RouteInform {
    explicit RouteInform(double total_time, const std::vector<RoutingItems> &routing_items)
    : total_time_(total_time)
    , routing_items_(routing_items) {
    }
    double total_time_;
    std::vector<RoutingItems> routing_items_;
};

double ComputeTimeForEdge(int bus_wait_time, double bus_velocity, int road_distance);

std::optional<RouteInform> GetRoutingItems(const graph::Router<double>& router, routing::RoutingSettings& rt, const Stop* from_ptr, const Stop* to_ptr);
}// namespace routing
}// namespace catalogue