#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>



namespace catalogue {
namespace head {
class TransportCatalogue;
}//namespase head
namespace stat {
enum class QueryTypeStat {
    StatBus,
    StatStop,
};

struct QueryStat {
    std::vector<std::pair<QueryTypeStat, std::string>> query_stat_;//vector string with query stats
};

std::istream& operator>>(std::istream &is, QueryStat &q);

void ReadQueryStat(std::istream& os, QueryStat& q);

struct StopsForBusStat {
    StopsForBusStat() = default;

    explicit StopsForBusStat(const std::tuple<std::string_view, int, int, uint64_t, double> stops_for_bus);

    StopsForBusStat(const StopsForBusStat &other);

    StopsForBusStat &operator=(const StopsForBusStat &rhs);

    bool Is_Empty() const;

    std::tuple<std::string_view, int, int, uint64_t, double> stops_for_bus_;
};

struct BusesForStopStat {
    BusesForStopStat() = default;

    explicit BusesForStopStat(const std::map<std::string_view, std::set<std::string_view>> buses_for_stop);

    BusesForStopStat(const BusesForStopStat &other);

    BusesForStopStat &operator=(const BusesForStopStat &rhs);

    bool Is_Empty() const;

    std::map<std::string_view, std::set<std::string_view>> buses_for_stop_;
};

struct GetInfoStat {
    std::vector<std::pair<StopsForBusStat, BusesForStopStat>> info_;
};

GetInfoStat GetStatInfo(const head::TransportCatalogue& tc, QueryStat& q);

StopsForBusStat GetStopsForBus(const head::TransportCatalogue& tc, const std::string& stat_bus);

BusesForStopStat GetBusesForStop(const head::TransportCatalogue& tc, const std::string& stat_stop);

std::ostream& operator<<(std::ostream& os, const GetInfoStat& r);

std::ostream& operator<<(std::ostream& os, const StopsForBusStat& r);

std::ostream& operator<<(std::ostream& os, const BusesForStopStat& r);
}//namespace stat
}//namespace catalogue