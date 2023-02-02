#include "request_handler.h"

namespace catalogue {
namespace stat {
RequestHandler::RequestHandler(const head::TransportCatalogue& db) : db_(db) {
}

const head::TransportCatalogue& RequestHandler::GetTransportCatalogue() const {
    return db_;
}

std::set<std::string_view> RequestHandler::GetStops() const {
    std::set<std::string_view> result;
    for (const auto& [key, val] : db_.stopname_to_stop_) {
        result.insert(key);
    }
    return result;
}

std::set<std::string_view> RequestHandler::GetBuses() const {
    std::set<std::string_view> result;
    for (const auto& [key, val] : db_.busname_to_bus_) {
        result.insert(key);
    }
    return result;
}

std::unordered_set<const Stop*> RequestHandler::GetStopsPtr() const {
    std::unordered_set<const Stop*> result;
    for (const auto& [key, val] : db_.stopname_to_stop_) {
        result.insert(val);
    }
    return result;
}

std::unordered_set<const Bus*> RequestHandler::GetBusesPtr() const {
    std::unordered_set<const Bus*> result;
    for (const auto& [key, val] : db_.busname_to_bus_) {
        result.insert(val);
    }
    return result;
}

const Stop* RequestHandler::FindStop(const std::string_view stop) const noexcept {
    const Stop* stop_ptr = nullptr;
    try {
        stop_ptr = GetStopPtr(stop);
    }
    catch (const std::invalid_argument &e) {
        return stop_ptr;
    }
    return stop_ptr;
}

const Bus* RequestHandler::FindBus(const std::string_view bus) const noexcept {
    const Bus* bus_ptr = nullptr;
    try {
        bus_ptr = GetBusPtr(bus);
    }
    catch (const std::invalid_argument &e) {
        return bus_ptr;
    }

    return bus_ptr;
}

const Stop& RequestHandler::GetStopIndex(const std::string_view stop) const {
    auto it_stop = db_.stopname_to_stop_.find(stop);
    using namespace std::literals;
    if (it_stop == db_.stopname_to_stop_.end()) {
        throw std::invalid_argument("Stop not found"s);
    }
    return *db_.stopname_to_stop_.at(stop);
}

const Stop* RequestHandler::GetStopPtr(const std::string_view stop) const {
    auto it_stop = db_.stopname_to_stop_.find(stop);
    using namespace std::literals;
    if (it_stop == db_.stopname_to_stop_.end()) {
        throw std::invalid_argument("Stop not found"s);
    }
    return db_.stopname_to_stop_.at(stop);
}

const Bus& RequestHandler::GetBusIndex(const std::string_view bus) const {
    auto it_bus = db_.busname_to_bus_.find(bus);
    using namespace std::literals;
    if (it_bus == db_.busname_to_bus_.end()) {
        throw std::invalid_argument("Bus not found"s);
    }
    return *db_.busname_to_bus_.at(bus);
}

const Bus* RequestHandler::GetBusPtr(const std::string_view bus) const {
    auto it_bus = db_.busname_to_bus_.find(bus);
    using namespace std::literals;
    if (it_bus == db_.busname_to_bus_.end()) {
        throw std::invalid_argument("Bus not found"s);
    }
    return db_.busname_to_bus_.at(bus);
}

std::vector<const Stop*> RequestHandler::GetBusInfoVec(const Bus* bus) const {
    if (bus) {
        return bus->GetBusAndStops();
    }
    return {};
}

std::vector<const Bus*> RequestHandler::GetStopInfoVec(const Stop* stop) const {
    if (stop) {
        return stop->GetStopAndBuses();
    }
    return {};
}

std::vector<const Stop*> RequestHandler::GetBusInfoVec(const std::string_view bus) const {
    const Bus* bus_ptr = FindBus(bus);
    if (bus_ptr)
    {
        return bus_ptr->GetBusAndStops();
    }
    return {};
}

std::vector<const Bus*> RequestHandler::GetStopInfoVec(const std::string_view stop) const {
    const Stop* stop_ptr = FindStop(stop);
    if (stop_ptr) {
        return stop_ptr->GetStopAndBuses();
    }
    return {};
}

std::unordered_set<const Stop*> RequestHandler::GetBusInfoSet(const Bus* bus) const {
    std::vector<const Stop*> res_vec = GetBusInfoVec(bus);
    std::unordered_set<const Stop *> result;
    result.insert(res_vec.begin(), res_vec.end());
    return result;
}

std::unordered_set<const Bus*> RequestHandler::GetStopInfoSet(const Stop* stop) const {
    std::vector<const Bus*> res_vec = GetStopInfoVec(stop);
    std::unordered_set<const Bus*> result;
    result.insert(res_vec.begin(), res_vec.end());
    return result;
}

std::vector<geo::Coordinates> RequestHandler::GetStopsForBusGeoCoordinates(const std::string_view bus) const {
    std::vector<geo::Coordinates> geo_coords;

    for (const auto& stop_ptr : GetBusInfoVec(bus)) {
        geo_coords.push_back(geo::Coordinates{stop_ptr->GetGeo()});
    }
    return geo_coords;
}

std::vector<geo::Coordinates> RequestHandler::GetFinalStopsForBusGeoCoordinates(const std::string_view bus) const {
    std::vector<geo::Coordinates> geo_coords;
    const Bus *bus_ptr = FindBus(bus);

    if (bus_ptr && !GetBusInfoVec(bus).empty()) {
        const Stop* last_stop = GetBusInfoVec(bus)[GetBusInfoVec(bus).size() - 1];
        const Stop *final_stop = GetStopPtr(bus_ptr->GetFinalStop());
        geo_coords.push_back(geo::Coordinates{last_stop->GetGeo()});

        if (!bus_ptr->IsRing() && last_stop->GetStop() != final_stop->GetStop()) {
            geo_coords.push_back(geo::Coordinates{final_stop->GetGeo()});
        }
    }
    return geo_coords;
}

std::vector<geo::Coordinates> RequestHandler::GetStopsGeoCoordinates() const {
    std::vector<geo::Coordinates> geo_coords;

    for (const auto stop : GetStops()) {
        if (!GetStopInfoVec(stop).empty()) {
            geo_coords.push_back(geo::Coordinates{GetStopPtr(stop)->GetGeo()});
        }
    }
    return geo_coords;
}

std::vector<std::string_view> RequestHandler::GetStopsName() const {
    std::vector<std::string_view> stops_name;

    for (const auto stop : GetStops()) {
        if (!GetStopInfoVec(stop).empty()) {
            stops_name.push_back(GetStopPtr(stop)->GetStop());
        }
    }

    return stops_name;
}

int RequestHandler::ComputeDistance(const Stop* from, const Stop* to) const {
    double road_distance = 0.0;
    auto it = db_.stops_distance_.find(std::make_pair(from, to));
    if (it == db_.stops_distance_.end()) {
        it = db_.stops_distance_.find(std::make_pair(to, from));
    }
    if (it != db_.stops_distance_.end()) {
        road_distance = it->second;
    }
    return road_distance;
}

StopsForBusStat GetStopsForBus(const RequestHandler& rh, const std::string_view name) {
    StopsForBusStat r;
    std::string_view str(name);
    const Bus* bus_stat_ptr = rh.FindBus(str);
    if (bus_stat_ptr) {
        int r_size = rh.GetBusInfoVec(bus_stat_ptr).size();
        int u_size = rh.GetBusInfoSet(bus_stat_ptr).size();;
        double l_route_geo = rh.GetTransportCatalogue().ComputeGeoDistance(bus_stat_ptr, r_size);
        int l_route_map = rh.GetTransportCatalogue().ComputeMapDistance(bus_stat_ptr, r_size);;
        double c_curvature = l_route_map / l_route_geo;

        r.stops_for_bus_ = std::make_tuple(r_size, u_size, l_route_map, c_curvature);
    }

    return r;
}

BusesForStopStat GetBusesForStop(const RequestHandler& rh, const std::string_view name) {
    BusesForStopStat r;
    std::string_view str(name);
    const Stop* stop_stat_ptr = rh.FindStop(str);
    if (stop_stat_ptr) { //проверяем есть ли такая остановка
        std::unordered_set<const Bus*> stop_info = rh.GetStopInfoSet(stop_stat_ptr);
        if (stop_info.empty()) {//проверяем есть ли у остановки маршруты
            using namespace std::literals;
            r.buses_for_stop_.insert("no buses"sv);
        } else {
            for (const auto& bus : stop_info) {;
                r.buses_for_stop_.insert(bus->GetBus());
            }
        }
    }
    return r;
}
}//namespace stat
}//namespace catalogue