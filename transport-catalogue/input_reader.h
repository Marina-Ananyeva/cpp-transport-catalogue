#pragma once

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

std::tuple<std::string_view, double, double> ParseQueryStop(std::string_view text);

std::vector<std::pair<std::pair<const std::string_view, const std::string_view>, int>> ParseQueryDistance(std::string_view text);

std::string_view ParseQueryBus(std::string_view text);

std::vector<std::string_view> MakeRoute(std::string_view str);
}//namespace input
}//namespace catalogue