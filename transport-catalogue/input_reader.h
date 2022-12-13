#pragma once

#include "log_duration.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <string>
#include <tuple>
#include <vector>

namespace catalogue {
namespace input {
enum class QueryInputType {
    NewStop,
    NewBus,
};

struct QueryInput {
    QueryInputType type;
    std::vector<std::string> text_stops_;//string with stop
    std::vector<std::string> text_buses_;//string with bus
};

std::istream& operator>>(std::istream &is, QueryInput &q);

int ReadLineWithNumber(std::istream& is);

void ReadQueryInput(std::istream& is, QueryInput& q);

head::TransportCatalogue::Stop ParseQueryStop(std::string_view text);

std::vector<std::pair<std::pair<const head::TransportCatalogue::Stop*, const head::TransportCatalogue::Stop*>, int>> ParseQueryDistance(head::TransportCatalogue& tc, std::string_view text);

head::TransportCatalogue::Bus ParseQueryBus(std::string_view text);

std::pair<const head::TransportCatalogue::Bus, std::vector<const head::TransportCatalogue::Stop *>> MakeRoute(head::TransportCatalogue &tc, std::string_view str);

void FillCatalogue(head::TransportCatalogue& tc, QueryInput& q, std::istream& is);

}//namespace input
}//namespace catalogue