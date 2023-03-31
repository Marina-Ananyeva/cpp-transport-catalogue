#pragma once

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "log_duration.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <string>
#include <tuple>
#include <vector>

namespace catalogue {
namespace reader {
using Stop = domain::Stop;
using Bus = domain::Bus;

struct Query {
    json::Array text_base_stops_;                               //вектор с запросами на добавление информации об остановках
    json::Array text_base_buses_;                               //вектор с запросами на добавление информации о маршрутах
    json::Array text_stat_;                                     //вектор с запросами на предоставление информации
    json::Dict text_render_settings_;                           //словарь с настройками визуализации карты
    json::Dict text_routing_settings_;                          //словарь с настройками маршрутизации
    json::Dict text_serialization_settings_;                    //словарь с настройками сериализации
};

class JSONReader {
public:
    void ParseQuery(std::istream&, Query&);

    Stop ParseQueryStop(const json::Dict&);

    std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> ParseQueryDistance(const stat::RequestHandler&, const json::Dict&);

    Bus ParseQueryBus(const json::Dict&);

    std::pair<const Bus*, std::vector<const Stop*>> ParseQueryBusRoute(const stat::RequestHandler&, const json::Dict&);

    svg::Color ReadColor(const json::Node&);

    void AddRenderSettings(renderer::RenderSettings&, const json::Dict&);

    void AddRoutingSettings(routing::RoutingSettings&, const json::Dict&);

    json::Dict MakeJsonDocStopsForBus(int, const stat::StopsForBusStat&);

    json::Dict MakeJsonDocBusesForStop(int, const stat::BusesForStopStat&);

    json::Dict MakeJsonDocForRoute(int, const std::optional<routing::RouteInform>&);
};

void FillCatalogue(head::TransportCatalogue&, Query&, renderer::RenderSettings&, renderer::MapObjects&, routing::RoutingSettings&, std::istream&);

void ReadRequest(head::TransportCatalogue&, Query&, std::istream&);

void ExecuteStatRequests(head::TransportCatalogue&, Query&, renderer::MapObjects&, routing::RoutingSettings&, std::ostream&);
}//namespace reader
}//namespace catalogue