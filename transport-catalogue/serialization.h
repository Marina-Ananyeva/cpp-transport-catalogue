#pragma once

#include "domain.h"
#include "graph.h"
#include "json_reader.h"
#include "log_duration.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <graph.pb.h>
#include <map_renderer.pb.h>
#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>
#include <svg.pb.h>

#include <iostream>
#include <fstream>

#include <unordered_map>
#include <unordered_set>

namespace catalogue {
namespace serialize {
using Stop = domain::Stop;
using Bus = domain::Bus;
using TransportCatalogue = head::TransportCatalogue;

using Stop_s = transport_catalogue::Stop;
using Bus_s = transport_catalogue::Bus;
using Distance_s = transport_catalogue::Distance;
using Route_s = transport_catalogue::Route;
using TransportCatalogue_s = transport_catalogue::Catalogue;

struct StopsAndBusesDictionary {
    std::unordered_map<std::string_view, int> stops_dict_s;
    std::unordered_map<std::string_view, int> buses_dict_s;
    std::unordered_map<int, std::string_view> stops_dict_d;
    std::unordered_map<int, std::string_view> buses_dict_d;
};

void SerializeStops(const TransportCatalogue&, TransportCatalogue_s&, StopsAndBusesDictionary&);
void SerializeDistance(const TransportCatalogue&, TransportCatalogue_s&, StopsAndBusesDictionary&);
void SerializeBusesAndRoute(const TransportCatalogue&, TransportCatalogue_s&, StopsAndBusesDictionary&);

Svg::Point SerializePointSvg(const svg::Point&);
Svg::Color SerializeColorSvg(const svg::Color&);
void SerializeRendererSetting(const TransportCatalogue&, const renderer::RenderSettings&, Map_renderer::MapRenderer&);

void SerializeRouterSetting(const routing::RoutingSettings&, Router::RouterSetting&);
void SerializeRouter(const routing::RoutingSettings&, Router::RouterSetting&, const StopsAndBusesDictionary&);
void SerializeGraph(const routing::RoutingSettings&, Graph::Graph&);

void SerializeTransportCatalogue(const TransportCatalogue&, const reader::Query&, const renderer::RenderSettings&, const renderer::MapObjects&, const routing::RoutingSettings&, std::ofstream&);

void FillStopsAfterDeserialize(const TransportCatalogue_s&, TransportCatalogue&, StopsAndBusesDictionary&);
void FillBusesAfterDeserialize(const TransportCatalogue_s&, TransportCatalogue&, StopsAndBusesDictionary&);
void FillDistanceAfterDeserialize(const TransportCatalogue_s&, TransportCatalogue&, const stat::RequestHandler&, StopsAndBusesDictionary&);
void FillRouteAfterDeserialize(const TransportCatalogue_s&, TransportCatalogue&, const stat::RequestHandler&, StopsAndBusesDictionary&);

svg::Color FillColorSvg(const Svg::Color&);
void FillRenderSetting(const Map_renderer::MapRenderer&, renderer::RenderSettings&);
void FillMapObject(const stat::RequestHandler&, renderer::RenderSettings&, renderer::MapObjects&);

void FillRouterSetting(const Router::RouterSetting&, routing::RoutingSettings&);
void FillRouter(const Router::RouterSetting&, routing::RoutingSettings&, const stat::RequestHandler&, const StopsAndBusesDictionary&);
void FillGraph(const Graph::Graph &, routing::RoutingSettings&);

void FillCatalogueAfterDeserialize(const TransportCatalogue_s&, TransportCatalogue&, renderer::RenderSettings&, renderer::MapObjects&, routing::RoutingSettings&);

void DeserializeTransportCatalogue(std::ifstream &, TransportCatalogue&, renderer::RenderSettings&, renderer::MapObjects&, routing::RoutingSettings&);
} // namespace serialize
} // namespace catalogue
