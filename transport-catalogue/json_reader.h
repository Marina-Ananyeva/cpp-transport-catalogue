#pragma once

#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "log_duration.h"
#include "map_renderer.h"
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
    json::Dict text_render_setting_;                            //вектор с настройками визуализации карты
};

std::istream& operator>>(std::istream &is, Query &q);

void ParseQuery(std::istream& is, Query& q);

Stop ParseQueryStop(const json::Dict& stops);

std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>> ParseQueryDistance(const stat::RequestHandler& rh, const json::Dict& stops);

Bus ParseQueryBus(const json::Dict& buses);

std::pair<const Bus*, std::vector<const Stop*>> ParseQueryBusRoute(const stat::RequestHandler& rh, const json::Dict& buses);

svg::Color ReadColor(const json::Node& color_setting);

void AddRenderSetting(renderer::RenderSetting& r, const json::Dict&& setting);

void FillCatalogue(head::TransportCatalogue& tc, Query& q, renderer::RenderSetting& r, renderer::MapObjects& m, std::istream& is);

json::Dict MakeJsonDocStopsForBus(const int query_id, const stat::StopsForBusStat &r);

json::Dict MakeJsonDocBusesForStop(const int query_id, const stat::BusesForStopStat &r);

void ExecuteStatRequests(head::TransportCatalogue& tc, Query& q, renderer::MapObjects& m, std::ostream& os);
}//namespace reader
}//namespace catalogue