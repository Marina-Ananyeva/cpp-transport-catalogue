#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace svg {
class Rgb {
public:
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue)
    : red(red)
    , green(green)
    , blue(blue) {
    }

    Rgb(int red, int green, int blue)
    : red(static_cast<uint8_t>(red))
    , green(static_cast<uint8_t>(green))
    , blue(static_cast<uint8_t>(blue)) {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

class Rgba {
public:
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
    : red(red)
    , green(green)
    , blue(blue)
    , opacity(opacity) {
    }

    Rgba(int red, int green, int blue, double opacity)
    : red(static_cast<uint8_t>(red))
    , green(static_cast<uint8_t>(green))
    , blue(static_cast<uint8_t>(blue))
    , opacity(opacity) {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    using namespace std::literals;
    inline const Color NoneColor = {"none"s};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream &operator<<(std::ostream &os, StrokeLineCap type);

std::ostream &operator<<(std::ostream &os, StrokeLineJoin type);

void PrintColor(std::ostream &os, std::monostate);

void PrintColor(std::ostream &os, std::string color);

void PrintColor(std::ostream &os, Rgb color);

void PrintColor(std::ostream &os, Rgba color);

std::ostream &operator<<(std::ostream &os, Color color);

template <typename Owner>
class PathProps {
public:
    Owner &SetFillColor(Color color);

    Owner &SetStrokeColor(Color color);

    Owner& SetStrokeWidth(double width);

    Owner& SetStrokeLineCap(StrokeLineCap line_cap);

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

    const std::optional<Color>& GetFillColor() const;

    const std::optional<Color>& GetStrokeColor() const;

    const std::optional<double>& GetStrokeWidth() const;

    const std::optional<StrokeLineCap>& GetStrokeLineCap() const;

    const std::optional<StrokeLineJoin>& GetStrokeLineJoin() const;

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream &out) const;

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;                               //цвет заливки
    std::optional<Color> stroke_color_;                             //цвет контура
    std::optional<double> stroke_width_;                            //толщина линии
    std::optional<StrokeLineCap> stroke_line_cap_;                  //тип формы конца линии
    std::optional<StrokeLineJoin> stroke_line_join_;                //тип формы соединения линий
};

template <typename Owner>
Owner& PathProps<Owner>::SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
    stroke_line_cap_ = line_cap;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
    stroke_line_join_ = line_join;
    return AsOwner();
}

template <typename Owner>
const std::optional<Color>& PathProps<Owner>::GetFillColor() const {
    if (fill_color_) {
        return fill_color_;
    }
    return std::nullopt;
}

template <typename Owner>
const std::optional<Color>& PathProps<Owner>::GetStrokeColor() const {
    if (stroke_color_) {
        return stroke_color_;
    }
    return std::nullopt;
}

template <typename Owner>
const std::optional<double>& PathProps<Owner>::GetStrokeWidth() const {
    if (stroke_width_) {
        return stroke_width_;
    }
    return std::nullopt;
}

template <typename Owner>
const std::optional<StrokeLineCap>& PathProps<Owner>::GetStrokeLineCap() const {
    if (stroke_line_cap_.has_value()) {
        return stroke_line_cap_.value();
    }
    return std::nullopt;
}

template <typename Owner>
const std::optional<StrokeLineJoin>& PathProps<Owner>::GetStrokeLineJoin() const {
    if (stroke_line_join_.has_value()) {
        return stroke_line_join_.value();
    }
    return std::nullopt;
}

template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
    using namespace std::literals;

    if (fill_color_) {
        out << " fill=\""sv << *fill_color_ << "\""sv;
    }
    if (stroke_color_) {
        out << " stroke=\""sv << *stroke_color_ << "\""sv;
    }
    if (stroke_width_) {
        out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
    }
    if (stroke_line_cap_) {
        out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
    }
    if (stroke_line_join_) {
        out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
    }
}

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }

    Point &SetPoint(double xx, double yy);

    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;
    Circle &SetCenter(Point center);

    Circle &SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;

    // Добавляет очередную вершину к ломаной линии
    Polyline &AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text &SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text &SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text &SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text &SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text &SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text &SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    Point position_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_ = {};
    std::string font_weight_ = {};
    std::string data_ = {};
};

class ObjectContainer {
public:
    ObjectContainer() = default;

    /*
     Метод Add добавляет в svg-документ любой объект типа T
     Пример использования:
     ObjectContainer container;
     container.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    template <typename T>
    void Add(T obj);

    virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

    virtual ~ObjectContainer() = default;
    
protected:
    std::vector<std::unique_ptr<Object>> objects_;
};

template <typename T>
void ObjectContainer::Add(T obj) {
    objects_.emplace_back(std::make_unique<T>(std::move(obj)));
}


class Document : public ObjectContainer {
public:
    Document() = default;

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object> &&obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};
} // namespace svg

namespace shapes {
    using namespace std::literals;
class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer &container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable {
public:
    explicit Star (svg::Point center, double outer_rad, double inner_rad, int num_rays) 
    : center_(center)
    , outer_rad_(outer_rad)
    , inner_rad_(inner_rad)
    , num_rays_(num_rays) {
    }

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const;

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer &container) const override;
private:
    svg::Point center_ = {0.0, 0.0};
    double outer_rad_ = 0.0;
    double inner_rad_ = 0.0;
    int num_rays_ = 0;
};

class Snowman : public svg::Drawable {
public:

    explicit Snowman (svg::Point center_head, double rad_head)
    : center_head_(center_head)
    , rad_head_(rad_head) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer &container) const override;
private:
    svg::Point center_head_ = {0.0, 0.0};
    double rad_head_ = 0.0;
};
}//namespace shapes

