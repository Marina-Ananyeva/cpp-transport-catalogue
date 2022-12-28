#pragma once

#include "constants.h"
#include "geo.h"
#include "request_handler.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <vector>

//inline const double EPSILON = 1e-6;

namespace catalogue {
namespace renderer {
bool IsZero(double value);

class SphereProjector {
public:
    SphereProjector() = default;
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);
    
    SphereProjector(const SphereProjector &other);
    SphereProjector &operator=(const SphereProjector &rhs);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
double padding_ = 0.0;
double min_lon_ = 0.0;
double max_lat_ = 0.0;
double zoom_coeff_ = 0.0;
};

class RenderSetting {
public:
    double width_ = 0.0;                    // ширина
    double height_ = 0.0;                   // высота изображения в пикселях
    double padding_ = 0.0;                  // отступ краёв карты от границ SVG-документа
    double line_width_ = 0.0;               // толщина линий, которыми рисуются автобусные маршруты
    double stop_radius_ = 0.0;              // радиус окружностей, которыми обозначаются остановки
    int bus_label_font_size_ = 0;           // размер текста, которым написаны названия автобусных маршрутов
    svg::Point bus_label_offset_;           // смещение надписи с названием маршрута относительно координат конечной остановки на карте
    int stop_label_font_size_ = 0;          // размер текста, которым отображаются названия остановок
    svg::Point stop_label_offset_;          // смещение названия остановки относительно её координат на карте
    svg::Color underlayer_color_;           // цвет подложки под названиями остановок и маршрутов
    double underlayer_width_ = 0.0;         // толщина подложки под названиями остановок и маршрутов
    std::vector<svg::Color> color_palette_; // цветовая палитра

    SphereProjector proj;                   //проектор географических координат на плоскость
};

//------------BusLine---------------------------------------------------
class BusLine : public svg::Drawable {
public:
    explicit BusLine(const svg::Color color, const std::vector<svg::Point>& stops_point, const RenderSetting &r_);
    void Draw(svg::ObjectContainer &container) const override;
    svg::Color color_;
    std::vector<svg::Point> stops_point_;
    const RenderSetting &r_;
};

//--------------BusText-------------------------------------------------
class BusText : public svg::Drawable {
public:
    explicit BusText(const svg::Color color, const std::vector<svg::Point>& final_stops_position, const std::string_view bus_name, const RenderSetting &r_);

    void Draw(svg::ObjectContainer &container) const override;

    svg::Color color_;
    std::vector<svg::Point> final_stops_position_;
    std::string bus_name_;
private:
    const RenderSetting &r_;
};

//----------------StopCircle-----------------------------------------------
class StopCircle : public svg::Drawable {
public:
    explicit StopCircle(const svg::Color color, const std::vector<svg::Point>& stops_point, const RenderSetting &r_);
    void Draw(svg::ObjectContainer &container) const override;

    svg::Color color_;
    std::vector<svg::Point> stops_point_;
    const RenderSetting &r_;
};

//-------------------StopText------------------------------------------------
class StopText : public svg::Drawable {
public:
    explicit StopText(const svg::Color color, const std::vector<svg::Point>& stops_position, const std::vector<std::string_view>& stops_name, const RenderSetting &r);

    void Draw(svg::ObjectContainer &container) const override;

    svg::Color color_;
    std::vector<svg::Point> stops_position_;
    std::vector<std::string_view> stops_name_;
private:
    const RenderSetting &r_;
};

//-------------------MapObjects-----------------------------------------------
class MapObjects {
public:
    std::vector<std::unique_ptr<svg::Drawable>> map_object_;        //вектор с объектами визуализации
    svg::Document map_object_detail_;                               //виктор с отрисованными объектами визуализации
};

svg::Color ChooseColor(const RenderSetting &r, const size_t number);

const SphereProjector& MakeSphereProjector(const stat::RequestHandler& rh, RenderSetting &r);

std::vector<svg::Point> MakeSphereProjectorStopsPoint(const std::vector<const domain::Stop*>& all_stops, const SphereProjector& proj);

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer &target);

template <typename Container>
void DrawPicture(const Container &container, svg::ObjectContainer &target);

void MapRenderer(const stat::RequestHandler& rh, RenderSetting &r, renderer::MapObjects& m);

std::ostream &PrintMap(std::ostream &os, const MapObjects &m);

std::ostream &operator<<(std::ostream &os, const MapObjects &m);
}// namespace renderer
}//namespace catalogue
