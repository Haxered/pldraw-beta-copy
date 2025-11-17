#include "expression.hpp"

#include <cmath>
#include <limits>
#include <cctype>
#include <sstream>
#include <tuple>

bool tol_eq(const double a, const double b) {
    constexpr double eps = std::numeric_limits<double>::epsilon();
    const double diff = std::fabs(a - b);
    return diff <= eps;
}

std::ostream &print_point(std::ostream &out, const Point &p) {
    out << "(" << p.x << "," << p.y << ")";
    return out;
}

std::ostream &print_rect(std::ostream &out, const Rect &r) {
    out << "(";
    print_point(out, r.point1) << ",";
    print_point(out, r.point2) << ")";
    return out;
}

Expression::Expression(const bool tf) {
    head.type = BooleanType;
    head.value.bool_value = tf;
}

Expression::Expression(const double num) {
    head.type = NumberType;
    head.value.num_value = num;
}

Expression::Expression(const std::string &sym) {
    head.type = SymbolType;
    head.value.sym_value = sym;
}

Expression::Expression(const std::tuple<double, double> &value) {
    head.type = PointType;
    head.value.point_value.x = std::get<0>(value);
    head.value.point_value.y = std::get<1>(value);
}

Expression::Expression(std::tuple<double, double> start,
                       std::tuple<double, double> end) {
    head.type = LineType;
    head.value.line_value.start.x = std::get<0>(start);
    head.value.line_value.start.y = std::get<1>(start);
    head.value.line_value.end.x = std::get<0>(end);
    head.value.line_value.end.y = std::get<1>(end);
}

Expression::Expression(std::tuple<double, double> center,
                       std::tuple<double, double> start,
                       double angle) {
    head.type = ArcType;
    head.value.arc_value.center.x = std::get<0>(center);
    head.value.arc_value.center.y = std::get<1>(center);
    head.value.arc_value.start.x = std::get<0>(start);
    head.value.arc_value.start.y = std::get<1>(start);
    head.value.arc_value.angle = angle;
}

Expression::Expression(const Point &point) {
    head.type = PointType;
    head.value.point_value = point;
}

Expression::Expression(const Line &line) {
    head.type = LineType;
    head.value.line_value = line;
}

Expression::Expression(const Arc &arc) {
    head.type = ArcType;
    head.value.arc_value = arc;
}

Expression::Expression(const Rect &rect) {
    head.type = RectType;
    head.value.rect_value = rect;
}

Expression::Expression(const FillRect &fillRect) {
    head.type = FillRectType;
    head.value.fill_rect_value = fillRect;
}

Expression::Expression(const Ellipse &ellipse) {
    head.type = EllipseType;
    head.value.ellipse_value = ellipse;
}


bool Expression::operator==(const Expression &exp) const noexcept {
    if (head.type != exp.head.type) {
        return false;
    }

    switch (head.type) {
        case NoneType:
            break;

        case BooleanType:
            if (head.value.bool_value != exp.head.value.bool_value) {
                return false;
            }
            break;

        case NumberType: {
            if (!tol_eq(head.value.num_value, exp.head.value.num_value)) {
                return false;
            }
            break;
        }

        case SymbolType:
            if (head.value.sym_value != exp.head.value.sym_value) {
                return false;
            }
            break;

        case PointType: {
            const Point &a = head.value.point_value, &b = exp.head.value.point_value;
            if (!tol_eq(a.x, b.x) || !tol_eq(a.y, b.y)) return false;
            break;
        }

        case LineType: {
            const Line &a = head.value.line_value, &b = exp.head.value.line_value;
            if (!tol_eq(a.start.x, b.start.x) || !tol_eq(a.start.y, b.start.y) ||
                !tol_eq(a.end.x, b.end.x) || !tol_eq(a.end.y, b.end.y))
                return false;
            break;
        }

        case ArcType: {
            const Arc &a = head.value.arc_value, &b = exp.head.value.arc_value;
            if (!tol_eq(a.center.x, b.center.x) || !tol_eq(a.center.y, b.center.y) ||
                !tol_eq(a.start.x, b.start.x) || !tol_eq(a.start.y, b.start.y) ||
                !tol_eq(a.angle, b.angle))
                return false;
            break;
        }

        case RectType: {
            const Rect &a = head.value.rect_value, &b = exp.head.value.rect_value;
            if (!tol_eq(a.point1.x, b.point1.x) || !tol_eq(a.point1.y, b.point1.y) ||
                !tol_eq(a.point2.x, b.point2.x) || !tol_eq(a.point2.y, b.point2.y))
                return false;
            break;
        }

        case FillRectType: {
            const FillRect &a = head.value.fill_rect_value, &b = exp.head.value.fill_rect_value;
            const Rect &ar = a.rect, &br = b.rect;
            if (!tol_eq(ar.point1.x, br.point1.x) || !tol_eq(ar.point1.y, br.point1.y) ||
                !tol_eq(ar.point2.x, br.point2.x) || !tol_eq(ar.point2.y, br.point2.y) ||
                !tol_eq(a.r, b.r) || !tol_eq(a.g, b.g) || !tol_eq(a.b, b.b))
                return false;
            break;
        }

        case EllipseType: {
            const Rect &a = head.value.ellipse_value.rect, &b = exp.head.value.ellipse_value.rect;
            if (!tol_eq(a.point1.x, b.point1.x) || !tol_eq(a.point1.y, b.point1.y) ||
                !tol_eq(a.point2.x, b.point2.x) || !tol_eq(a.point2.y, b.point2.y))
                return false;
            break;
        }
    }

    // TODO: check if this is even required
    if (tail.size() != exp.tail.size()) return false;
    for (std::size_t i = 0; i < tail.size(); ++i) {
        if (!(tail[i] == exp.tail[i])) return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &out, const Expression &exp) {
    const Atom &h = exp.head;

    switch (h.type) {
        case NoneType:
            out << "()";
            break;

        case BooleanType:
            out << "(" << (h.value.bool_value ? "True" : "False") << ")";
            break;

        case NumberType:
            out << "(" << h.value.num_value << ")";
            break;

        case SymbolType:
            out << "(" << h.value.sym_value << ")";
            break;

        case PointType:
            print_point(out, h.value.point_value);
            break;

        case LineType:
            out << "(";
            print_point(out, h.value.line_value.start) << ",";
            print_point(out, h.value.line_value.end) << ")";
            break;

        case ArcType:
            out << "(";
            print_point(out, h.value.arc_value.center) << ",";
            print_point(out, h.value.arc_value.start) << " " << h.value.arc_value.angle << ")";
            break;

        case RectType:
            print_rect(out, h.value.rect_value);
            break;

        case FillRectType:
            out << "(";
            print_rect(out, h.value.fill_rect_value.rect);
            out << " (" << h.value.fill_rect_value.r << "," << h.value.fill_rect_value.g << "," << h.value.
                    fill_rect_value.b << "))";
            break;

        case EllipseType:
            out << "(";
            print_rect(out, h.value.ellipse_value.rect);
            out << ")";
            break;
    }
    return out;
}

bool token_to_atom(const std::string &token, Atom &atom) {
    if (token == "True") {
        atom.type = BooleanType;
        atom.value.bool_value = true;
        return true;
    }
    if (token == "False") {
        atom.type = BooleanType;
        atom.value.bool_value = false;
        return true;
    }

    // Geometry constructors should be symbols
    if (token == "point" || token == "line" || token == "arc" ||
        token == "rect" || token == "fill_rect" || token == "ellipse") {
        atom.type = SymbolType;
        atom.value.sym_value = token;
        return true;
    }

    // Number
    std::istringstream iss(token);
    iss.setf(std::ios::fmtflags(0), std::ios::basefield);
    double temp;
    if ((iss >> temp) && iss.rdbuf()->in_avail() == 0) {
        atom.type = NumberType;
        atom.value.num_value = temp;
        return true;
    }

    // Symbol (must not start with a digit)
    if (!token.empty() && std::isdigit(static_cast<unsigned char>(token[0])) == 0) {
        atom.type = SymbolType;
        atom.value.sym_value = token;
        return true;
    }

    return false;
}
