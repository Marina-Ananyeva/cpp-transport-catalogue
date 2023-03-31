#include "domain.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace catalogue {
namespace domain {
Stop::Stop(const std::string& stop) : stop_(stop) {
}

Stop::Stop(const std::tuple<std::string&, double, double> info_stop) : stop_(std::get<0>(info_stop)), geo_({std::get<1>(info_stop), std::get<2>(info_stop)}) {
}

Stop::Stop(const std::string& stop, const double latitude, const double longitude) : stop_(stop), geo_({latitude, longitude}) {
}

Stop::Stop(const Stop& other) {
    this->stop_ = other.stop_;
    this->geo_ = other.geo_;
    this->stop_and_buses_ = other.stop_and_buses_;
}

Stop& Stop::operator=(const Stop& rhs) {
    if (&rhs != this) {
        this->stop_ = rhs.stop_;
        this->geo_ = rhs.geo_;
        this->stop_and_buses_ = rhs.stop_and_buses_;
    }
    return *this;
}

bool Stop::operator==(const Stop &rhs) {
    if (this->stop_ == rhs.stop_ 
        && this->geo_ == rhs.geo_ 
        && this->stop_and_buses_ == rhs.stop_and_buses_ ) {
        return true;
    }
    return false;
}

std::string_view Stop::GetStop() const {
    return stop_;
}

std::pair<double, double> Stop::GetGeo() const {
    return geo_;
}

std::vector<const Bus*> Stop::GetStopAndBuses() const {
    return stop_and_buses_;
}

void Stop::AddBusFromRoute(const Bus* bus) const {
    stop_and_buses_.push_back(bus);
}

Bus::Bus(const std::string& bus) : bus_(bus) {
}

Bus::Bus(const std::string& bus, bool is_ring) : bus_(bus), is_ring_(is_ring) {
}

Bus::Bus(const Bus& other) {
    this->bus_ = other.bus_;
    this->bus_and_stops_ = other.bus_and_stops_;
    this->is_ring_ = other.is_ring_;
    this->final_stop_ = other.final_stop_;
}

Bus& Bus::operator=(const Bus& rhs) {
    if (&rhs != this) {
        this->bus_ = rhs.bus_;
        this->bus_and_stops_ = rhs.bus_and_stops_;
        this->is_ring_ = rhs.is_ring_;
        this->final_stop_ = rhs.final_stop_;
    }
    return *this;
}

bool Bus::operator==(const Bus &rhs) {
    if (this->bus_ == rhs.bus_ && this->bus_and_stops_ == rhs.bus_and_stops_ 
        && this->is_ring_ == rhs.is_ring_ && this->final_stop_ == rhs.final_stop_) {
        return true;
    }
    return false;
}

std::string_view Bus::GetBus() const {
    return bus_;
}

std::string_view Bus::GetFinalStop() const {
    return final_stop_;
}

std::vector<const Stop*> Bus::GetBusAndStops() const {
    return bus_and_stops_;
}

bool Bus::IsRing() const {
    return is_ring_;
}

void Bus::AddStopForRoute(const Stop* stop) const {
    bus_and_stops_.push_back(stop);
}

void Bus::AddFinalStop(const Stop* stop) const {
    final_stop_ = stop->GetStop();
}
}//namespace domain
}//namespace catalogue