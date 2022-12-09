#pragma once

#include <iostream>
#include <string>
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

int ReadLineWithNumber();

void ReadQueryInput(QueryInput& q);
}
}