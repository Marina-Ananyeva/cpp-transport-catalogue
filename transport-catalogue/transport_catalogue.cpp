#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace catalogue {
namespace head {
void TransportCatalogue::AddSerializeFileName(const std::string &file_name) {
    serialize_file_name_ = file_name;
}

void TransportCatalogue::AddStop(const Stop new_stop) {
    stops_.push_back(new_stop);
}

void TransportCatalogue::AddStopDirectory() {
    for (auto& stop : stops_) {
        stopname_to_stop_.insert({stop.GetStop(), &stop});
    }
}

void TransportCatalogue::AddDistance(const std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>>& stops_distance) {
    for (const auto& [stops, dist] : stops_distance) {
        stops_distance_.insert({std::make_pair(stops.first, stops.second), dist});
    }
}

void TransportCatalogue::AddBus(const Bus new_bus) {
    buses_.push_back(new_bus);
}

void TransportCatalogue::AddBusDirectory() {
    for (auto& bus : buses_) {
        busname_to_bus_.insert({bus.GetBus(), &bus});
    }
}

void TransportCatalogue::AddRoute(std::pair<const Bus*, std::vector<const Stop*>>& bus_route) {
   const Bus *bus_ptr = bus_route.first;
    bus_ptr->AddFinalStop(bus_route.second[bus_route.second.size() - 1]); //добавляем последнюю остановку из вектора как конечную остановку маршрута

    if (!bus_ptr->IsRing()) {
     //добавляем остановки до кольцевого маршрута
        std::vector<const Stop*> copy_vec(bus_route.second.size() - 1);//создаем вектор - буфер размером на 1 меньше (минус последний элемент)
        std::reverse_copy(bus_route.second.begin(), prev(bus_route.second.end()), copy_vec.begin());//копируем элементы в обратном порядке кроме последнего
        bus_route.second.reserve(bus_route.second.size() * 2 - 1);
        bus_route.second.insert(bus_route.second.end(), copy_vec.begin(), copy_vec.end());
    }

    //заполняем вектор остановок для маршрута и добавляем маршрут в вектор маршрутов для каждой остановки из маршрута
    for (size_t i = 0; i < bus_route.second.size(); ++i) {
        bus_ptr->AddStopForRoute(bus_route.second[i]);

        const Stop *stop_ptr = bus_route.second[i];
        std::vector<const Bus *> vec = stop_ptr->GetStopAndBuses();
        if (count(vec.begin(), vec.end(), bus_ptr) == 0) {
            stop_ptr->AddBusFromRoute(bus_ptr); 
        }
    }
}

double TransportCatalogue::ComputeGeoDistance(const Bus* bus_ptr, const int route_size) const {
    double l_route_geo = 0.0;
    for (auto i = 0; i < route_size - 1; ++i) {
        Stop from = *(bus_ptr->GetBusAndStops()[i]);
        Stop to = *(bus_ptr->GetBusAndStops()[i + 1]);
        l_route_geo += geo::ComputeDistance(geo::Coordinates(from.GetGeo()), geo::Coordinates(to.GetGeo()));
    }

    return l_route_geo;
}

int TransportCatalogue::ComputeMapDistance(const Bus* bus_ptr, const int route_size) const {
    uint64_t l_route_map = 0;
    for (auto i = 0; i < route_size - 1; ++i) {
        const Stop* from = (bus_ptr->GetBusAndStops()[i]);
        const Stop* to = (bus_ptr->GetBusAndStops()[i + 1]);

        auto it = stops_distance_.find(std::make_pair(from, to));
        if (it == stops_distance_.end()) {
            it = stops_distance_.find(std::make_pair(to, from));
        }
        int distance = it -> second;
        l_route_map += distance;
    }
    return l_route_map;
}

double TransportCatalogue::ComputeCurvature(const double l_route_geo, const int l_route_map) const {
    double c_curature = l_route_map / l_route_geo;
    return c_curature;
}

std::string TransportCatalogue::GetSerializeFileName() const {
    return serialize_file_name_;
}
}//namespace head
}//namespace catalogue