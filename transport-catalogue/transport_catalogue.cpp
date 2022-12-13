#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace catalogue {
namespace head {
TransportCatalogue::Stop::Stop(const std::string_view stop) : stop_(stop) {
}

TransportCatalogue::Stop::Stop(const std::tuple<std::string_view, double, double> info_stop) : stop_(std::get<0>(info_stop)), geo_({std::get<1>(info_stop), std::get<2>(info_stop)}) {
}

TransportCatalogue::Stop::Stop(const std::string_view stop, const double latitude, const double longitude) : stop_(stop), geo_({latitude, longitude}) {
}

TransportCatalogue::Stop::Stop(const TransportCatalogue::Stop& other) {
    this->stop_ = other.stop_;
    this->geo_ = other.geo_;
    this->stop_and_buses_ = other.stop_and_buses_;
}

TransportCatalogue::Stop& TransportCatalogue::Stop::operator=(const TransportCatalogue::Stop& rhs) {
    if (&rhs != this) {
        this->stop_ = rhs.stop_;
        this->geo_ = rhs.geo_;
        this->stop_and_buses_ = rhs.stop_and_buses_;
    }
    return *this;
}

bool TransportCatalogue::Stop::operator==(const TransportCatalogue::Stop &rhs) {
    if (this->stop_ == rhs.stop_ && this->geo_ == rhs.geo_ && this->stop_and_buses_ == rhs.stop_and_buses_) {
        return true;
    }
    return false;
}

std::string_view TransportCatalogue::Stop::GetStop() const {
    return stop_;
}

std::pair<double, double> TransportCatalogue::Stop::GetGeo() const {
    return geo_;
}

TransportCatalogue::Bus::Bus(const std::string_view bus) : bus_(bus) {
}

TransportCatalogue::Bus::Bus(const TransportCatalogue::Bus& other) {
    this->bus_ = other.bus_;
    this->bus_and_stops_ = other.bus_and_stops_;
}

TransportCatalogue::Bus& TransportCatalogue::Bus::operator=(const TransportCatalogue::Bus& rhs) {
    if (&rhs != this) {
        this->bus_ = rhs.bus_;
        this->bus_and_stops_ = rhs.bus_and_stops_;
    }
    return *this;
}

bool TransportCatalogue::Bus::operator==(const TransportCatalogue::Bus &rhs) {
    if (this->bus_ == rhs.bus_ && this->bus_and_stops_ == rhs.bus_and_stops_) {
        return true;
    }
    return false;
}

std::string_view TransportCatalogue::Bus::GetBus() const {
    return bus_;
}

void TransportCatalogue::AddStop(const Stop& new_stop) {
    stops_.push_back(new_stop);
}

void TransportCatalogue::AddStopDirectory() {
    for (auto& stop : stops_) {
        stopname_to_stop_.insert({stop.GetStop(), &stop});
    }
}

void TransportCatalogue::AddDistance(const std::vector<std::pair<std::pair<const head::TransportCatalogue::Stop*, const head::TransportCatalogue::Stop*>, int>>& stops_distance) {
    for (const auto& [stops, dist] : stops_distance) {
        stops_distance_.insert({std::make_pair(stops.first, stops.second), dist});
    }
}

void TransportCatalogue::AddBus(const Bus& new_bus) {
    buses_.push_back(new_bus);
}

void TransportCatalogue::AddBusDirectory() {
    for (auto& bus : buses_) {
        busname_to_bus_.insert({bus.GetBus(), &bus});
    }
}
void TransportCatalogue::AddRoute(const std::pair<const head::TransportCatalogue::Bus, std::vector<const head::TransportCatalogue::Stop*>>& bus_route) {
    auto it = find(buses_.begin(), buses_.end(), bus_route.first);
    for (size_t i = 0; i < bus_route.second.size(); ++i) {
        it->bus_and_stops_.push_back(bus_route.second[i]);//fill bus and stops

        Stop *stop_ptr = const_cast<Stop *>(bus_route.second[i]);
        const Bus *bus_ptr = GetBusPtr(it->GetBus());
        if (count(stop_ptr->stop_and_buses_.begin(), stop_ptr->stop_and_buses_.end(), bus_ptr) == 0) {
            stop_ptr->stop_and_buses_.push_back(bus_ptr); // fill stop and buses
        }
    }
}

void TransportCatalogue::CompleteCatalogue() {
    for (auto& bus : buses_) {
        bus_to_stops_[&bus].insert(bus.bus_and_stops_.begin(), bus.bus_and_stops_.end());
    }

    for (auto& stop : stops_) {
        stop_to_buses_[&stop].insert(stop.stop_and_buses_.begin(), stop.stop_and_buses_.end());
    }
}

const TransportCatalogue::Stop* TransportCatalogue::FindStop(const std::string_view stop) const {
    const Stop* stop_ptr = nullptr;
    try {
        stop_ptr = GetStopPtr(stop);
    }
    catch (const std::invalid_argument &e) {
        return stop_ptr;
    }
    return stop_ptr;
}

const TransportCatalogue::Bus* TransportCatalogue::FindBus(const std::string_view bus) const {
    const Bus* bus_ptr = nullptr;
    try {
        bus_ptr = GetBusPtr(bus);
    }
    catch (const std::invalid_argument &e) {
        return bus_ptr;
    }

    return bus_ptr;
}

std::vector<const TransportCatalogue::Stop*> TransportCatalogue::GetBusInfoVec(const head::TransportCatalogue::Bus* bus) const {
    if (bus) {
        return bus->bus_and_stops_;
    }
    return {};
}

std::unordered_set<const head::TransportCatalogue::Bus*> TransportCatalogue::GetStopInfoSet(const TransportCatalogue::Stop* stop) const {
    if (stop) {
        return stop_to_buses_.at(stop);
    }
    return {};
}

std::unordered_set<const TransportCatalogue::Stop*> TransportCatalogue::GetBusInfoSet(const head::TransportCatalogue::Bus* bus) const {
    if (bus) {
        return bus_to_stops_.at(bus);
    }
    return {};
}

const TransportCatalogue::Stop& TransportCatalogue::GetStopIndex(const std::string_view stop) const {
    auto it_stop = stopname_to_stop_.find(stop);
    using namespace std::literals;
    if (it_stop == stopname_to_stop_.end()) {
        throw std::invalid_argument("Stop not found"s);
    }
    return *stopname_to_stop_.at(stop);
}

const TransportCatalogue::Stop* TransportCatalogue::GetStopPtr(const std::string_view stop) const {
    auto it_stop = stopname_to_stop_.find(stop);
    using namespace std::literals;
    if (it_stop == stopname_to_stop_.end()) {
        throw std::invalid_argument("Stop not found"s);
    }
    return stopname_to_stop_.at(stop);
}

const TransportCatalogue::Bus& TransportCatalogue::GetBusIndex(const std::string_view bus) const {
    auto it_bus = busname_to_bus_.find(bus);
    using namespace std::literals;
    if (it_bus == busname_to_bus_.end()) {
        throw std::invalid_argument("Bus not found"s);
    }
    return *busname_to_bus_.at(bus);
}

const TransportCatalogue::Bus* TransportCatalogue::GetBusPtr(const std::string_view bus) const {
    auto it_bus = busname_to_bus_.find(bus);
    using namespace std::literals;
    if (it_bus == busname_to_bus_.end()) {
        throw std::invalid_argument("Bus not found"s);
    }
    return busname_to_bus_.at(bus);
}
double TransportCatalogue::ComputeGeoDistance(const std::string_view bus, const int route_size) const {
    double l_route_geo = 0.0;
    for (auto i = 0; i < route_size - 1; ++i) {
        const Bus *bus_ptr = GetBusPtr(bus);
        Stop from = *(bus_ptr->bus_and_stops_[i]);
        Stop to = *(bus_ptr->bus_and_stops_[i + 1]);
        l_route_geo += geo::ComputeDistance(geo::Coordinates(from.GetGeo()), geo::Coordinates(to.GetGeo()));
    }

    return l_route_geo;
}

uint64_t TransportCatalogue::ComputeMapDistance(const std::string_view bus, const int route_size) const {
    uint64_t l_route_map = 0;
    for (auto i = 0; i < route_size - 1; ++i) {
        const Bus *bus_ptr = GetBusPtr(bus);
        const Stop* from = (bus_ptr->bus_and_stops_[i]);
        const Stop* to = (bus_ptr->bus_and_stops_[i + 1]);

        auto it = stops_distance_.find(std::make_pair(from, to));
        if (it == stops_distance_.end()) {
            it = stops_distance_.find(std::make_pair(to, from));
        }
        uint64_t distance = it -> second;
        l_route_map += distance;
    }
    return l_route_map;
}

double TransportCatalogue::ComputeCurvature(const double l_route_geo, const uint64_t l_route_map) const {
    double c_curature = l_route_map / l_route_geo;
    return c_curature;
}
}//namespace head
}//namespace catalogue