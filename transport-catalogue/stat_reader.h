#pragma once

#include "input_reader.h"

#include <iostream>
#include <string>
#include <vector>

namespace catalogue {
namespace stat {
enum class QueryTypeStat {
    StatBus,
    StatStop,
};

struct QueryStat {
    std::vector<std::pair<QueryTypeStat, std::string>> query_stat_;//vector string with query stats
};

std::istream& operator>>(std::istream &is, QueryStat &q);

void ReadQueryStat(QueryStat& q);
}
}