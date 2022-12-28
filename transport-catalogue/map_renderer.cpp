#include "map_renderer.h"

namespace catalogue {
namespace renderer {
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
}

SphereProjector::SphereProjector(const SphereProjector &other) {
    this->padding_ = other.padding_;
    this->min_lon_ = other.min_lon_;
    this->max_lat_ = other.max_lat_;
    this->zoom_coeff_ = other.zoom_coeff_;
}

SphereProjector& SphereProjector::operator=(const SphereProjector &rhs) {
    if (&rhs != this) {
        this->padding_ = rhs.padding_;
        this->min_lon_ = rhs.min_lon_;
        this->max_lat_ = rhs.max_lat_;
        this->zoom_coeff_ = rhs.zoom_coeff_;
    }
    return *this;
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

//-----------------BusLine--------------------------------
BusLine::BusLine(const svg::Color color, const std::vector<svg::Point>& stops_point, const RenderSetting &r) 
    : color_(color)
    , stops_point_(stops_point)
    , r_(r) {
}

void BusLine::Draw(svg::ObjectContainer &container) const {
    svg::Polyline bus_line;
    bus_line
            .SetFillColor(svg::NoneColor)
            .SetStrokeColor(color_)
            .SetStrokeWidth(r_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for (const auto& point : stops_point_) {
        bus_line.AddPoint(point);
    }
    container.Add(bus_line);
}

//-----------------BusText-----------------------------
BusText::BusText(const svg::Color color, const std::vector<svg::Point>& final_stops_position, const std::string_view bus_name, const RenderSetting &r) 
    : color_(color)
    , final_stops_position_(final_stops_position)
    , bus_name_(bus_name)
    , r_(r) {
}

void BusText::Draw(svg::ObjectContainer &container) const {  
    for (const auto& pos : final_stops_position_) {
        using namespace std::literals;

        svg::Text bus_text_back;
        bus_text_back
                .SetFillColor(r_.underlayer_color_)
                .SetStrokeColor(r_.underlayer_color_)
                .SetStrokeWidth(r_.underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition(pos)
                .SetOffset(r_.bus_label_offset_)
                .SetFontSize(r_.bus_label_font_size_)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus_name_);

        container.Add(bus_text_back);

        svg::Text bus_text_name;
        bus_text_name
                .SetFillColor(color_)
                .SetPosition(pos)
                .SetOffset(r_.bus_label_offset_)
                .SetFontSize(r_.bus_label_font_size_)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus_name_);

        container.Add(bus_text_name);
    }
}

//----------------StopCircle----------------------
StopCircle::StopCircle(const svg::Color color, const std::vector<svg::Point>& stops_point, const RenderSetting &r) 
    : color_(color)
    , stops_point_(stops_point)
    , r_(r) {
}

void StopCircle::Draw(svg::ObjectContainer &container) const {
    for (const auto& point : stops_point_) {
        svg::Circle stop_circle;
        stop_circle
                .SetFillColor(color_)
                .SetCenter(point)
                .SetRadius(r_.stop_radius_);

        container.Add(stop_circle);
    }
}

//-------------------StopText-----------------------------
StopText::StopText(const svg::Color color, const std::vector<svg::Point>& stops_position, const std::vector<std::string_view>& stops_name, const RenderSetting &r)
    : color_(color)
    , stops_position_(stops_position)
    , stops_name_(stops_name)
    , r_(r) {
}

void StopText::Draw(svg::ObjectContainer &container) const {
    const size_t count = stops_position_.size();

    for (size_t i = 0; i < count; ++i) {
        using namespace std::literals;
        std::string stop_name(stops_name_[i]);

        svg::Text bus_text_back;
        bus_text_back
                .SetFillColor(r_.underlayer_color_)
                .SetStrokeColor(r_.underlayer_color_)
                .SetStrokeWidth(r_.underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition(stops_position_[i])
                .SetOffset(r_.stop_label_offset_)
                .SetFontSize(r_.stop_label_font_size_)
                .SetFontFamily("Verdana"s)
                .SetData(stop_name);

        container.Add(bus_text_back);

        svg::Text stop_text_name;
        stop_text_name
                .SetFillColor(color_)
                .SetPosition(stops_position_[i])
                .SetOffset(r_.stop_label_offset_)
                .SetFontSize(r_.stop_label_font_size_)
                .SetFontFamily("Verdana"s)
                .SetData(stop_name);

        container.Add(stop_text_name);
    }
}

//-------------------------------------------------------------------------------
svg::Color ChooseColor(const RenderSetting& r, const size_t number) {
    if (number < r.color_palette_.size()) {
        return r.color_palette_[number];
    }
    return r.color_palette_[number % r.color_palette_.size()];
}

const SphereProjector& MakeSphereProjector(const stat::RequestHandler& rh, RenderSetting& r) {
    const double WIDTH = r.width_;
    const double HEIGHT = r.height_;
    const double PADDING = r.padding_;

    // Определяем все точки, подлежащие проецированию - ВСЕ остановки, через которые проходят маршруты
    std::vector<geo::Coordinates> geo_coords; 

    for (const auto stop : rh.GetStops()) {
        if (!rh.GetStopInfoVec(stop).empty()) {
            geo_coords.push_back(geo::Coordinates{rh.GetStopPtr(stop)->GetGeo()});
        }
    }
    // Создаём проектор сферических координат на карту
    SphereProjector proj{geo_coords.begin(), geo_coords.end(), WIDTH, HEIGHT, PADDING};
    r.proj = proj;

    return r.proj;
}

std::vector<svg::Point> MakeSphereProjectorStopsPoint(const std::vector<geo::Coordinates>& geo_coords, const SphereProjector& proj) {
    std::vector<svg::Point> result;

    // Проецируем координаты
    for (const auto geo_coord: geo_coords) {
        const svg::Point screen_coord = proj(geo_coord);
        result.push_back(screen_coord);
    }
    return result;
}

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}

void MapRenderer(const stat::RequestHandler& rh, RenderSetting& r, renderer::MapObjects& m) {
    MakeSphereProjector(rh, r); // создаем проектор

    size_t number = 0;//счетчик порядка маршрута

    for (const auto bus : rh.GetBuses()) {
        svg::Color color{ChooseColor(r, number)};//определяем цвет линии маршрута

        if (!rh.GetBusInfoVec(rh.GetBusPtr(bus)).empty()) {
            std::vector<svg::Point> stops_point = MakeSphereProjectorStopsPoint(rh.GetStopsForBusGeoCoordinates(bus), r.proj);//определяем координаты остановок каждого маршрута
            m.map_object_.emplace_back(std::make_unique<BusLine>(color, stops_point, r));//создаем обЪект визуализации линию маршрута

            ++number;
        }
    }

    number = 0;//счетчик порядка маршрута
    for (const auto bus : rh.GetBuses()) {
        svg::Color color{ChooseColor(r, number)};//определяем цвет линии маршрута

        if (!rh.GetBusInfoVec(rh.GetBusPtr(bus)).empty()) {
            std::vector<svg::Point> final_stops_point = MakeSphereProjectorStopsPoint(rh.GetFinalStopsForBusGeoCoordinates(bus), r.proj);//определяем координаты final остановок каждого маршрута
            m.map_object_.emplace_back(std::make_unique<BusText>(color, final_stops_point, bus, r)); // создаем обЪект визуализации название маршрута

            ++number;
        }
    }

    {
        using namespace std::literals;
        svg::Color color("white"s);
        std::vector<svg::Point> stops_point = MakeSphereProjectorStopsPoint(rh.GetStopsGeoCoordinates(), r.proj);//определяем координаты остановок каждого маршрута
        m.map_object_.emplace_back(std::make_unique<StopCircle>(color, stops_point, r));//создаем обЪект визуализации линию маршрута
    }

    {
        using namespace std::literals;
        svg::Color color("black"s);
        std::vector<svg::Point> stops_point = MakeSphereProjectorStopsPoint(rh.GetStopsGeoCoordinates(), r.proj);//определяем координаты остановок каждого маршрута
        m.map_object_.emplace_back(std::make_unique<StopText>(color, stops_point, rh.GetStopsName(), r));//создаем обЪект визуализации линию маршрута
    }

    DrawPicture(m.map_object_, m.map_object_detail_);//отрисовываем каждый объект визуализации
}

std::ostream& PrintMap(std::ostream& os, const MapObjects& m) {
    m.map_object_detail_.Render(os);//выводим в SVG-формате
    return os;
}

std::ostream& operator<<(std::ostream& os, const MapObjects& m) {
    PrintMap(os, m);
    return os;
}
}//namespace catalogue
}//namespace renderer