#pragma once

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace catalogue {
namespace domain {
struct Bus;
struct Stop {
public:
    Stop() = default;
    explicit Stop(const std::string& stop);
    explicit Stop(const std::tuple<std::string&, double, double> info_stop);
    explicit Stop(const std::string& stop, const double latitude, const double longitude);

    Stop(const Stop &other);
    Stop &operator=(const Stop &rhs);
    bool operator==(const Stop &rhs);

    std::string_view GetStop() const;                               //возвращает название остановки
    std::pair<double, double> GetGeo() const;                       //возвращает географические координаты остановки
    std::vector<const Bus*> GetStopAndBuses() const;                //возвращает вектор с маршрутами, которые проходят через остановку
    void AddBusFromRoute(const Bus* bus) const;                     //добавление маршрута, который проходит через остановку

private:
    std::string stop_;                                              //название остановки
    std::pair<double, double> geo_= {0.0, 0.0};                     //географические координаты остановки
    mutable std::vector<const Bus*> stop_and_buses_;                //вектор с указателями на маршруты, которые проходят через остановку
};

struct Bus {
public:
    Bus() = default;
    explicit Bus(const std::string& bus);
    explicit Bus(const std::string& bus, bool is_ring);

    Bus(const Bus &other);
    Bus &operator=(const Bus &rhs);
    bool operator==(const Bus &rhs);

    std::string_view GetBus() const;                                //возвращает название маршрута
    std::string_view GetFinalStop() const;                          //возвращает конечную остановку маршрута
    std::vector<const Stop*> GetBusAndStops() const;                //возвращает вектор с остановками, через которые проходит маршрут
    bool IsRing() const;                                            //возвращает значение кольцевой маршрут
    void AddStopForRoute(const Stop* stop) const;                   //добавление остановки, через которую проходит маршрут
    void AddFinalStop(const Stop *stop) const;                      //добавление конечной остановки

private:
    std::string bus_;                                               //название маршрута
    bool is_ring_;                                                  //признак кольцевой маршрут
    mutable std::vector<const Stop*> bus_and_stops_;                //вектор указателей на остановки, через которые проходит маршрут
    mutable std::string_view final_stop_;                           //конечная остановка
};
}//namespace domain
}//namespace catalogue