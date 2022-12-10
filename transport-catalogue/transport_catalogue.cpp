#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace catalogue {
namespace head {
void TransportCatalogue::AddStop(const std::vector<std::string>& text_stops) {
    if (!text_stops.empty()) {
        for (std::string_view str : std::move(text_stops)) {
            const Stop new_stop(input::ParseQueryStop(str));
            stops_.push_back(new_stop);
        }

        for (auto& stop : stops_) {
            stopname_to_stop_.insert({stop.GetStop(), &stop});
        }

        for (std::string_view str : std::move(text_stops)) {
            std::vector<std::pair<std::pair<const std::string_view, const std::string_view>, int>> stops_distance = input::ParseQueryDistance(str);
            for (const auto& [stops, dist] : stops_distance) {
                stops_distance_.insert({std::make_pair(GetStopPtr(stops.first), GetStopPtr(stops.second)), dist});
            }
        }
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

std::vector<std::string_view> TransportCatalogue::GetStopInfo(const std::string_view stop) const {
    std::vector<std::string_view> stop_info;
    const Stop* stop_stat_ptr = GetStopPtr(stop);
    for (const auto &bus : buses_) {
        for (const auto& stop_ptr : bus.bus_and_stops_) {
            if (stop_ptr == stop_stat_ptr && count(stop_info.begin(), stop_info.end(), bus.GetBus()) == 0) {
                stop_info.push_back(bus.GetBus());
            }
        }
    }
    return stop_info;
}

void TransportCatalogue::AddBus(const std::vector<std::string>& text_buses) {
    if (!text_buses.empty()) {
        for (std::string_view str : std::move(text_buses)) {
            Bus new_bus (input::ParseQueryBus(str));
            buses_.push_back(new_bus);
        }

        for (std::string_view str : std::move(text_buses)) {
            std::vector<std::string_view> bus_route = input::MakeRoute(str);
            Bus new_bus(bus_route[0]);
            auto it = find(buses_.begin(), buses_.end(), new_bus);
            for (size_t i = 1; i < bus_route.size(); ++i) {
                it->bus_and_stops_.push_back(GetStopPtr(bus_route[i]));
            }
        }

        for (auto& bus : buses_) {
            busname_to_bus_.insert({bus.GetBus(), &bus});
        }
    }
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

std::vector<std::string_view> TransportCatalogue::GetBusInfo(const std::string_view bus) const {
    std::vector<std::string_view> bus_info;
    for (const auto& stop : GetBusIndex(bus).bus_and_stops_) {
        bus_info.push_back(stop->GetStop());
    }
    return bus_info;
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

TransportCatalogue::Stop::Stop(const std::string_view stop) : stop_(stop) {
}

TransportCatalogue::Stop::Stop(const std::tuple<std::string_view, double, double> info_stop) : stop_(std::get<0>(info_stop)), geo_({std::get<1>(info_stop), std::get<2>(info_stop)}) {
}

TransportCatalogue::Stop::Stop(const std::string_view stop, const double latitude, const double longitude) : stop_(stop), geo_({latitude, longitude}) {
}

TransportCatalogue::Stop::Stop(const TransportCatalogue::Stop& other) {
    this->stop_ = other.stop_;
    this->geo_ = other.geo_;
}

TransportCatalogue::Stop& TransportCatalogue::Stop::operator=(const TransportCatalogue::Stop& rhs) {
    if (&rhs != this) {
        this->stop_ = rhs.stop_;
        this->geo_ = rhs.geo_;
    }
    return *this;
}

bool TransportCatalogue::Stop::operator==(const TransportCatalogue::Stop &rhs) {
    return (this->stop_ == rhs.stop_);
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
    return (this->bus_ == rhs.bus_);
}

std::string_view TransportCatalogue::Bus::GetBus() const {
    return bus_;
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
}//namespace head
}//namespace catalogue