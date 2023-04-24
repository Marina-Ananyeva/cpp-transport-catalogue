#pragma once

#include "domain.h"
#include "graph.h"
#include"ranges.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace catalogue {
namespace stat {
using Stop = domain::Stop;
using Bus = domain::Bus;

class RequestHandler {
public:
    RequestHandler(const head::TransportCatalogue& db);

    const head::TransportCatalogue &GetTransportCatalogue() const;                      //возвращает копию транспортного справочника

    std::set<std::string_view> GetStops() const;                                        //возвращает список всех остановок
    std::set<std::string_view> GetBuses() const;                                        //возвращает список всех маршрутов

    std::unordered_set<const Stop*> GetStopsPtr() const;                                //возвращает список указателей всех остановок
    std::unordered_set<const Bus*> GetBusesPtr() const;                                 //возвращает список указателей всех маршрутов

    const Stop* FindStop(const std::string_view stop) const noexcept;                   //возвращает указатель на остановку по названию 
                                                                                        //(не выбрасывает исключение, если не найдено возвращает nullptr)
    const Bus* FindBus(const std::string_view bus) const noexcept;                      //возвращает указатель на маршрут по названию
                                                                                        //(не выбрасывает исключение, если не найдено возвращает nullptr)
    const Stop& GetStopIndex(const std::string_view stop) const;                        //возвращает ссылку на остановку по названию 
                                                                                        //(выбрасывает исключение, если не найдено)
    const Stop* GetStopPtr(const std::string_view stop) const;                          //возвращает ссылку на маршрут по названию
                                                                                        //(выбрасывает исключение, если не найдено)
    const Bus& GetBusIndex(const std::string_view bus) const;                           //возвращает указатель на остановку по названию 
                                                                                        //(выбрасывает исключение, если не найдено)
    const Bus* GetBusPtr(const std::string_view bus) const;                             //возвращает указатель на маршрут по названию
                                                                                        //(не выбрасывает исключение, если не найдено)

    std::vector<const Stop*> GetBusInfoVec(const Bus* bus) const;                       //возвращает вектор с указателями на остановки по указателю на маршрут
    std::vector<const Bus*> GetStopInfoVec(const Stop* stop) const;                     //возвращает вектор с указателями на маршруты по указателю на остановку
    std::vector<const Stop*> GetBusInfoVec(const std::string_view bus) const;           //возвращает вектор с указателями на остановки по названию маршрута
    std::vector<const Bus*> GetStopInfoVec(const std::string_view stop) const;          //возвращает вектор с указателями на маршруты по названию остановки

    std::unordered_set<const Stop*> GetBusInfoSet(const Bus* bus) const;                //возвращает set с указателями на маршруты по указателю на остановку
    std::unordered_set<const Bus*> GetStopInfoSet(const Stop* stop) const;              //возвращает set с указателями на остановки по указателю на маршрут

    std::vector<geo::Coordinates> GetStopsForBusGeoCoordinates(const std::string_view bus) const;       //возвращает координаты остановок по непустому маршруту
    std::vector<geo::Coordinates> GetFinalStopsForBusGeoCoordinates(const std::string_view bus) const;  //возвращает координаты конечных остановок по непустому маршруту

    std::vector<geo::Coordinates> GetStopsGeoCoordinates() const;                       //возвращает координаты всех остановок, через которые проходят маршруты
    std::vector<std::string_view> GetStopsName() const;                                 //возвращает названия всех остановок, через которые проходят маршруты

    int ComputeDistance(const Stop* from, const Stop* to) const;                        //возвращает расстояние между остановками по справочнику расстояний

private:
    // RequestHandler использует агрегацию объекта "Транспортный Справочник"
    const head::TransportCatalogue &db_;
};

struct StopsForBusStat {
    StopsForBusStat() = default;

    std::tuple<int, int, int, double> stops_for_bus_;
};

struct BusesForStopStat {
    BusesForStopStat() = default;

    std::set<std::string_view> buses_for_stop_;
};

StopsForBusStat GetStopsForBus(const RequestHandler& rh, const std::string_view name);

BusesForStopStat GetBusesForStop(const RequestHandler& rh, const std::string_view name);
}//namespace stat
}//namespace catalogue