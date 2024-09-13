#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <variant>
#include <sstream>
#include <iomanip>

namespace svg {
    struct Rgb {
        Rgb(uint8_t r, uint8_t g, uint8_t b)
                : red(r), green(g), blue(b) {
        }
        Rgb() = default;
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba {
        Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
                : red(r), green(g), blue(b), opacity(op) {
        }
        Rgba() = default;
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;

    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{"none"};

    struct OstreamColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none";
        }
        void operator()(std::string str) {
            out << str;
        }
        void operator()(Rgb rgb) {
            out << "rgb("
                << std::to_string(rgb.red) << ','
                << std::to_string(rgb.green) << ','
                << std::to_string(rgb.blue)
                << ')';
        }
        void operator()(Rgba rgba) {
            out << "rgba("
                << std::to_string(rgba.red) << ','
                << std::to_string(rgba.green) << ','
                << std::to_string(rgba.blue) << ','
                << rgba.opacity
                << ')';
        }
    };


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

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& linecap);
    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& linejoin);

    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {

        /*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */

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

    class Object {
        /*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        std::optional<Color> GetFillColor() {
            return fill_color_;
        }

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                //std::ostringstream strm;

                out << " fill=\""sv;
                std::visit(OstreamColorPrinter{out}, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(OstreamColorPrinter{out}, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    class Circle final : public Object, public PathProps<Circle> {
        /*
         * Класс Circle моделирует элемент <circle> для отображения круга
         * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
         */
    public:
        Circle() = default;
        Circle(Point center, double radius) : center_(center), radius_(radius) {}
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = {0.0, 0.0};
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
        // Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
        // https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
    public:
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;
        std::string points_ = {};
    };

    class Text final : public Object, public PathProps<Text> {
        /*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    public:

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_ = {0.0, 0.0};
        Point offset_ = {0.0, 0.0};
        uint32_t size_ = 1;
        std::string weight_ = {};
        std::string family_ = {};
        std::string data_ = {};
    };

    class ObjectContainer {
    public:
        template<typename T>
        void Add(T obj);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        std::deque<std::shared_ptr<Object>> objects_;
        ~ObjectContainer() = default;
    };

    template<typename T>
    void ObjectContainer::Add(T obj) {
        objects_.emplace_back(std::make_unique<T>(std::move(obj)));
    }

    class Document : public ObjectContainer {
    public:
        Document() = default;

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
        /*private:
            std::deque<std::shared_ptr<Object>> objects_;*/
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };




}  // namespace svg