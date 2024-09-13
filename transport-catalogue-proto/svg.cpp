#include "svg.h"

#include <iomanip>
#include <cmath>

namespace svg {
    using namespace std::literals;

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& linecap) {
        if (linecap == StrokeLineCap::BUTT) {
            os << "butt";
        } else if (linecap == StrokeLineCap::ROUND) {
            os << "round";
        } else if (linecap == StrokeLineCap::SQUARE) {
            os << "square";
        }
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& linejoin) {
        if (linejoin == StrokeLineJoin::ARCS) {
            os << "arcs";
        } else if (linejoin == StrokeLineJoin::BEVEL) {
            os << "bevel";
        } else if (linejoin == StrokeLineJoin::MITER) {
            os << "miter";
        } else if (linejoin == StrokeLineJoin::MITER_CLIP) {
            os << "miter-clip";
        } else if (linejoin == StrokeLineJoin::ROUND) {
            os << "round";
        }
        return os;
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


// ---------- Polyline ------------------
//point.x = std::round(point.x / 0.01) * 0.01;
    Polyline& Polyline::AddPoint(Point point) {
        if (!points_.empty()) {
            points_ += ' ';
        }

        std::stringstream stream;
        stream << point.x << ',' << point.y;
        /*double intpart_x;
        if (modf(point.x, &intpart_x) == 0.0) {
            stream << intpart_x << ',';
        } else {
            stream << std::fixed << std::setprecision(4) << point.x << ',';
        }

        double intpart_y;
        if (modf(point.y, &intpart_y) == 0.0) {
            stream << intpart_y;
        } else {
            stream << std::fixed << std::setprecision(4) << point.y;
        }*/

        points_ += stream.str();
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv << points_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }


// ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" "sv;
        out << "dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;

        if (!family_.empty()) {
            out << "font-family=\""sv << family_ << "\""sv;
        }

        if (!weight_.empty()) {
            out << " font-weight=\""sv << weight_ << "\""sv;
        }

        out << ">"sv << data_ << "<"sv;

        out << "/text>"sv;
    }


// ---------- ObjectContainer ------------------



// ---------- Document ------------------

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext ctx(out, 2, 2);
        for (const auto& obj : objects_) {
            (*obj).Render(ctx);
        }

        out << "</svg>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }


// ---------- Drawable ------------------



}  // namespace svg