#pragma once

#include <utility>

namespace catalogue {
namespace geo {
struct Coordinates {
    explicit Coordinates(std::pair<double, double> geo);
    explicit Coordinates(double lat, double lng);
    double lat;                                             //широта
    double lng;                                             //долгота
    bool operator==(const Coordinates &other) const;
    bool operator!=(const Coordinates &other) const;
};

double ComputeDistance(Coordinates from, Coordinates to);   //рассчитывает расстояние по географическим координатам
}// namespace geo
}//namespace catalogue
