#define _USE_MATH_DEFINES 
#include "svg.h"

#include <cmath>

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& os, StrokeLineCap type) {
    switch(type) {
        using namespace std::literals;
        case StrokeLineCap::BUTT:
            os << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            os << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"s;
            break;
    }
    return os;
}

void PrintColor(std::ostream &os, std::monostate) {
    os << "none"s;
}

void PrintColor(std::ostream &os, std::string color) {
    os << color;
}

void PrintColor(std::ostream &os, Rgb color) {
    os << "rgb("s << int(color.red) << ","s << int(color.green) << ","s << int(color.blue) << ")"s;
}

void PrintColor(std::ostream &os, Rgba color) {
    os << "rgba("s << int(color.red) << ","s << int(color.green) << ","s << int(color.blue) << ","s << color.opacity << ")"s;
}

std::ostream &operator<<(std::ostream &os, Color color) {
    std::visit([&os](auto value)
               { PrintColor(os, value); },
               color);
    return os;
}

std::ostream& operator<<(std::ostream& os, StrokeLineJoin type) {
    switch(type) {
        using namespace std::literals;
            case StrokeLineJoin::ARCS: 
                os << "arcs"s;
                break;
            case StrokeLineJoin::BEVEL:
                os << "bevel"s;
                break;
            case StrokeLineJoin::MITER: 
                os << "miter"s;
                break;
            case StrokeLineJoin::MITER_CLIP: 
                os << "miter-clip"s;
                break;
            case StrokeLineJoin::ROUND:
                os << "round"s;
                break;
    }
    return os;
}

Point& Point::SetPoint (double xx, double yy) {
        x = xx;
        y = yy;
        return *this;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//------------- Polyline -------------
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (size_t i = 0; i < points_.size(); ++i) {
        out << points_[i].x << ","sv << points_[i].y;
        if ((i + 1) != points_.size()) {
            out << " "sv;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//------------- Text -------------
// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
    for (const auto& c : data_) {
        if (c == '\"') {
            out << "&quot;"sv;
            continue;
        }
        if (c == '\'') {
            out << "&apos;"sv;
            continue;
        }
        if (c == '<') {
            out << "&lt;"sv;
            continue;
        }
        if (c == '>') {
            out << "&gt;"sv;
            continue;
        }
        if (c == '&') {
            out << "&amp;"sv;
            continue;
        }
        out << c;
    }
    out << "</text>"sv;
}

//------------- Document -------------
// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    ObjectContainer::objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext context (out, 2, 2);

    for (const auto& doc : ObjectContainer::objects_) {
        doc->Render(context);
    }

    //out << "</svg>"sv << std::endl;
    out << "</svg>"sv;
}

}  // namespace svg

namespace shapes {
// Реализует метод Draw интерфейса svg::Drawable для Triangle
void Triangle::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

svg::Polyline Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)}).SetFillColor("red"s).SetStrokeColor("black"s);
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)}).SetFillColor("red"s).SetStrokeColor("black"s);
    }
    return polyline;
}

// Реализует метод Draw интерфейса svg::Drawable для Star
void Star::Draw(svg::ObjectContainer& container) const {
    container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_));
}

// Реализует метод Draw интерфейса svg::Drawable для Snowman
void Snowman::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Circle()
                            .SetCenter(svg::Point{center_head_.x, center_head_.y + 5 * rad_head_})
                            .SetRadius(rad_head_ * 2.0)
                            .SetFillColor("rgb(240,240,240)"s)
                            .SetStrokeColor("black"s));
    container.Add(svg::Circle()
                            .SetCenter(svg::Point{center_head_.x, center_head_.y + 2 * rad_head_})
                            .SetRadius(rad_head_ * 1.5)
                            .SetFillColor("rgb(240,240,240)"s)
                            .SetStrokeColor("black"s));
    container.Add(svg::Circle()
                            .SetCenter(center_head_)
                            .SetRadius(rad_head_)
                            .SetFillColor("rgb(240,240,240)"s)
                            .SetStrokeColor("black"s));
}
}//namespace shapes