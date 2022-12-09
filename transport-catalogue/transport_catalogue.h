#pragma once

#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

void TestTransportCatalogueAddStop();
void TestTransportCatalogueAddDistance();
void TestTransportCatalogueAddBus();
void TestTransportCatalogueGetIndexAndPtr();
void TestTransportCatalogueCompleteCatalogue();

namespace catalogue {
namespace head{
class TransportCatalogue{
friend void ::TestTransportCatalogueAddStop();
friend void ::TestTransportCatalogueAddDistance();
friend void ::TestTransportCatalogueAddBus();
friend void ::TestTransportCatalogueCompleteCatalogue();
friend void ::TestTransportCatalogueGetIndexAndPtr();

public:
    struct StopsForBusStat {
        StopsForBusStat() = default;

        explicit StopsForBusStat(std::tuple<std::string_view, int, int, uint64_t, double> stops_for_bus);

        StopsForBusStat(const StopsForBusStat &other);

        StopsForBusStat &operator=(const StopsForBusStat &rhs);

        bool Is_Empty() const;

        std::tuple<std::string_view, int, int, uint64_t, double> stops_for_bus_;
    };

    struct BusesForStopStat {
        BusesForStopStat() = default;

        explicit BusesForStopStat(std::map<std::string_view, std::set<std::string_view>> buses_for_stop);

        BusesForStopStat(const BusesForStopStat &other);

        BusesForStopStat &operator=(const BusesForStopStat &rhs);

        bool Is_Empty() const;

        std::map<std::string_view, std::set<std::string_view>> buses_for_stop_;
    };

    struct GetInfoStat {
        std::vector<std::pair<StopsForBusStat, BusesForStopStat>> info_;
    };

    void AddStop(const std::vector<std::string>& text_stops);

    void AddDistance(const std::vector<std::string>& text_stops);

    void AddBus(const std::vector<std::string>& text_buses);

    void CompleteCatalogue();

    GetInfoStat GetInfo (stat::QueryStat& q);

private:
    struct Stop {
    friend void ::TestTransportCatalogueAddStop();
    friend void ::TestTransportCatalogueGetIndexAndPtr();

    public:
        Stop() = default;

        explicit Stop(std::string_view stop, double latitude, double longitude);

        Stop(const Stop &other);

        Stop &operator=(const Stop &rhs);

        std::string_view GetStop() const;

        std::pair<double, double> GetGeo() const;

    private:
        std::string_view stop_;//name of stop
        std::pair<double, double> geo_= {0.0, 0.0};//coordinates of stop
    };

    struct PairStopPtrHash {
        std::size_t operator()(const std::pair<TransportCatalogue::Stop*, TransportCatalogue::Stop*>& s) const {
            uint64_t hash = (size_t)(s.first) * 37 + (size_t)(s.second) * 37 * 37;
            return static_cast<size_t>(hash);
        }
    };

    struct Bus {
    friend void ::TestTransportCatalogueAddBus();
    friend void ::TestTransportCatalogueGetIndexAndPtr();
    public:
        Bus() = default;

        explicit Bus(std::string_view bus);

        explicit Bus(std::string_view bus, bool is_ring_bus);

        Bus(const Bus &other);

        Bus &operator=(const Bus &rhs);

        std::string_view GetBus() const;

        bool IsRingBus() const;

        std::vector<Stop*> bus_and_stops_;//vector with pointers to stops for this bus
    private:
        std::string_view bus_;//name of bus
        bool is_ring_bus_ = false;//is ring route
    };

    Stop ParseQueryStop(std::string_view text);

    void MakeRoute(Bus& bus, std::string_view str);

    std::vector<std::pair<std::pair<Stop*, Stop*>, int>> ParseQueryDistance(std::string_view text);

    Bus ParseQueryBus(std::string_view text);

    const Stop& GetStopIndex(const std::string_view stop);

    Stop* GetStopPtr(const std::string_view stop);

    const Bus& GetBusIndex(const std::string_view bus);

    Bus* GetBusPtr(const std::string_view bus);

    StopsForBusStat GetStopsForBus(const std::string& stat_bus);

    BusesForStopStat GetBusesForStop(const std::string& stat_stop);

    std::deque<Stop> stops_;//all stops
    std::deque<Bus> buses_;//all busses

    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;//map with stop name to stop-pointer
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;//map with bus name to bus-pointer

    std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_buses_;//map with pointers to stop-buses
    std::unordered_map<const Bus*, std::unordered_set<const Stop*>> bus_to_stops_;//map with pointers to bus-stops

    std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopPtrHash> stops_distance_;//map with distance between stops
};
}
namespace detail {
std::ostream& operator<<(std::ostream& os, const head::TransportCatalogue::GetInfoStat& r);

std::ostream& operator<<(std::ostream& os, const head::TransportCatalogue::StopsForBusStat& r);

std::ostream& operator<<(std::ostream& os, const head::TransportCatalogue::BusesForStopStat& r);
}
}