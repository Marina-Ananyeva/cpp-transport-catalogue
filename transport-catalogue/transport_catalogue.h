#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <memory>
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
class TransportCatalogue{
using Stop = domain::Stop;
using Bus = domain::Bus;

public:
    void AddStop(const Stop& new_stop);                                                             //добавление остановки
    void AddStopDirectory();                                                                        //добавление словаря остановок
    void AddDistance(const std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>>& stops_distance);       //добавление расстояний
    void AddBus(const Bus& new_bus);                                                                //добавление названия маршрута
    void AddBusDirectory();                                                                         //добавление словаря маршрутов
    void AddRoute(std::pair<const Bus*, std::vector<const Stop*>>& bus_route);                      //добавление остановок маршрута

    double ComputeGeoDistance(const Bus* bus_ptr, const int route_size) const;                      //расчет географического расстояния
    int ComputeMapDistance(const Bus* bus_ptr, const int route_size) const;                         //расчет расстояния по справочнику расстояний
    double ComputeCurvature(const double l_route_geo, const int l_route_map) const;                 //расчет соотношения

    struct PairStopPtrHasher {
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& s) const {
            uint64_t hash = (size_t)(s.first) * 37 + (size_t)(s.second) * 37 * 37;
            return static_cast<size_t>(hash);
        }
    };

    std::deque<Stop> stops_;                                                                            //все остановки
    std::deque<Bus> buses_;                                                                             //все маршруты

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;                                //словарь название остановки - указатель
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;                                   //словарь название маршрута - указатель

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopPtrHasher> stops_distance_;    //словарь с расстоянием между остановками
};
}//namespace head
}//namespace catalogue