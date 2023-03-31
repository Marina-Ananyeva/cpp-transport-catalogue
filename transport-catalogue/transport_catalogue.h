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
    void AddSerializeFileName(const std::string &);                                             //добавление названия файла для сериализации
    void AddStop(const Stop);                                                                  //добавление остановки
    void AddStopDirectory();                                                                    //добавление словаря остановок
    void AddDistance(const std::vector<std::pair<std::pair<const Stop*, const Stop*>, int>>&);  //добавление расстояний
    void AddBus(const Bus);                                                                    //добавление названия маршрута
    void AddBusDirectory();                                                                     //добавление словаря маршрутов
    void AddRoute(std::pair<const Bus*, std::vector<const Stop*>>&);                            //добавление остановок маршрута

    double ComputeGeoDistance(const Bus*, const int ) const;                                    //расчет географического расстояния
    int ComputeMapDistance(const Bus*, const int) const;                                        //расчет расстояния по справочнику расстояний
    double ComputeCurvature(const double, const int) const;                                     //расчет соотношения

    std::string GetSerializeFileName() const;                                                   //возвращает название файла для сериализации
    
    struct PairStopPtrHasher {
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& s) const {
            uint64_t hash = (size_t)(s.first) * 37 + (size_t)(s.second) * 37 * 37;
            return static_cast<size_t>(hash);
        }
    };

    std::string serialize_file_name_;                                                                   //файл для сериализации
    std::deque<Stop> stops_;                                                                            //все остановки
    std::deque<Bus> buses_;                                                                             //все маршруты

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;                                //словарь название остановки - указатель
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;                                   //словарь название маршрута - указатель

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopPtrHasher> stops_distance_;    //словарь с расстоянием между остановками
};
}//namespace head
}//namespace catalogue