#pragma once

#include "geo.h"

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
void TestTransportCatalogueCompleteCatalogue();
void TestTransportCatalogueGetIndexAndPtr();


namespace catalogue {
namespace head {
class TransportCatalogue{
friend void ::TestTransportCatalogueAddStop();
friend void ::TestTransportCatalogueAddDistance();
friend void ::TestTransportCatalogueAddBus();
friend void ::TestTransportCatalogueCompleteCatalogue();
friend void ::TestTransportCatalogueGetIndexAndPtr();

public:
    struct Bus;
    struct Stop;

public:
    struct Stop {
    public:
        Stop() = default;
        explicit Stop(const std::string_view stop);
        explicit Stop(const std::tuple<std::string_view, double, double> info_stop);
        explicit Stop(const std::string_view stop, const double latitude, const double longitude);

        Stop(const Stop &other);
        Stop &operator=(const Stop &rhs);
        bool operator==(const Stop &rhs);

        std::string_view GetStop() const;
        std::pair<double, double> GetGeo() const;

        std::vector<const Bus*> stop_and_buses_;//vector with pointers to buses for this stop
    private:
        std::string_view stop_;//name of stop
        std::pair<double, double> geo_= {0.0, 0.0};//coordinates of stop
    };

    struct Bus {
    public:
        Bus() = default;
        explicit Bus(const std::string_view bus);

        Bus(const Bus &other);
        Bus &operator=(const Bus &rhs);
        bool operator==(const Bus &rhs);

        std::string_view GetBus() const;

        std::vector<const Stop*> bus_and_stops_;//vector with pointers to stops for this bus
    private:
        std::string_view bus_;//name of bus
    };

    void AddStop(const Stop& new_stop);

    void AddStopDirectory();

    void AddDistance(const std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>>& stops_distance);

    void AddBus(const Bus& new_bus);

    void AddBusDirectory();

    void AddRoute(const std::pair<const Bus, std::vector<const Stop*>>& bus_route);

    void CompleteCatalogue();

    const Stop* FindStop(const std::string_view stop) const;
    const Bus* FindBus(const std::string_view bus) const;

    std::vector<const Stop*> GetBusInfoVec(const Bus* bus) const;
    std::unordered_set<const Bus*> GetStopInfoSet(const Stop* stop) const;
    std::unordered_set<const Stop*> GetBusInfoSet(const Bus* bus) const;

    const Stop& GetStopIndex(const std::string_view stop) const;
    const Stop* GetStopPtr(const std::string_view stop) const;
    const Bus& GetBusIndex(const std::string_view bus) const;
    const Bus* GetBusPtr(const std::string_view bus) const;

    double ComputeGeoDistance(const std::string_view bus, const int route_size) const;
    uint64_t ComputeMapDistance(const std::string_view bus, const int route_size) const;
    double ComputeCurvature(const double l_route_geo, const uint64_t l_route_map) const;

private:
    struct PairStopPtrHasher {
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& s) const {
            uint64_t hash = (size_t)(s.first) * 37 + (size_t)(s.second) * 37 * 37;
            return static_cast<size_t>(hash);
        }
    };

    std::deque<Stop> stops_;//all stops
    std::deque<Bus> buses_;//all busses

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;//map with stop name to stop-pointer
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;//map with bus name to bus-pointer

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopPtrHasher> stops_distance_;//map with distance between stops

    std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_buses_;//map with pointers to stop-buses
    std::unordered_map<const Bus*, std::unordered_set<const Stop*>> bus_to_stops_;//map with pointers to bus-stops


};
}//namespace head
}//namespace catalogue