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

namespace catalogue {
namespace head {
class TransportCatalogue{
friend void ::TestTransportCatalogueAddStop();
friend void ::TestTransportCatalogueAddDistance();
friend void ::TestTransportCatalogueAddBus();
friend void ::TestTransportCatalogueGetIndexAndPtr();

private:
    struct Stop;
    struct Bus;

public:
    void AddStop(const std::vector<std::string>& text_stops);
    const Stop* FindStop(const std::string_view stop) const;
    std::vector<std::string_view> GetStopInfo(const std::string_view bus) const;   

    void AddBus(const std::vector<std::string>& text_buses);
    const Bus* FindBus(const std::string_view bus) const;
    std::vector<std::string_view> GetBusInfo(const std::string_view bus) const;

    double ComputeGeoDistance(const std::string_view bus, const int route_size) const;
    uint64_t ComputeMapDistance(const std::string_view bus, const int route_size) const;

private:
    struct Stop {
    friend void ::TestTransportCatalogueAddStop();
    friend void ::TestTransportCatalogueGetIndexAndPtr();

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

    private:
        std::string_view stop_;//name of stop
        std::pair<double, double> geo_= {0.0, 0.0};//coordinates of stop
    };

    struct PairStopPtrHash {
        std::size_t operator()(const std::pair<const TransportCatalogue::Stop*, const TransportCatalogue::Stop*>& s) const {
            uint64_t hash = (size_t)(s.first) * 37 + (size_t)(s.second) * 37 * 37;
            return static_cast<size_t>(hash);
        }
    };

    struct Bus {
    friend void ::TestTransportCatalogueAddBus();
    friend void ::TestTransportCatalogueGetIndexAndPtr();
    public:
        Bus() = default;
        explicit Bus(const std::string_view bus);

        Bus(const Bus &other);
        Bus &operator=(const Bus &rhs);
        bool operator==(const TransportCatalogue::Bus &rhs);

        std::string_view GetBus() const;

        std::vector<const Stop*> bus_and_stops_;//vector with pointers to stops for this bus
    private:
        std::string_view bus_;//name of bus
    };

    const Stop& GetStopIndex(const std::string_view stop) const;
    const Stop* GetStopPtr(const std::string_view stop) const;

    const Bus& GetBusIndex(const std::string_view bus) const;
    const Bus* GetBusPtr(const std::string_view bus) const;

    std::deque<Stop> stops_;//all stops
    std::deque<Bus> buses_;//all busses

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;//map with stop name to stop-pointer
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;//map with bus name to bus-pointer

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopPtrHash> stops_distance_;//map with distance between stops
};
}//namespace head
}//namespace catalogue