#ifndef TYPES_HPP
#define TYPES_HPP

// system includes
#include <tuple>
#include <ostream>
#include <string>
#include <utility>
#include <vector>


enum Type {
    NoneType, BooleanType, NumberType, SymbolType,
    PointType, LineType, ArcType, RectType, FillRectType, EllipseType
};

// Base Types
typedef bool Boolean;
typedef double Number;
typedef std::string Symbol;

// Geometric Types
struct Point {
    Number x;
    Number y;
};

struct Line {
    Point start;
    Point end;
};

struct Arc {
    Point center;
    Point start;
    Number angle; // rad
};

struct Rect {
    Point point1;
    Point point2;
};

// The rectangles' border should not be drawn (use Qt::transparent for the border)
struct FillRect {
    Rect rect;
    Number r;
    Number g;
    Number b;
};

struct Ellipse {
    Rect rect; // the smallest rectangle that can contain the Ellipse
};

struct Value {
    Boolean bool_value;
    Number num_value;
    Symbol sym_value;

    Point point_value;
    Line line_value;
    Arc arc_value;
    Rect rect_value;
    FillRect fill_rect_value;
    Ellipse ellipse_value;
};


struct Atom {
    Type type;
    Value value;
};


class Expression {
public:
    // Default construct an Expression of type None
    Expression() : head{NoneType} {
    };

    // Construct an Expression with a single Boolean atom with value
    explicit Expression(bool tf);

    // Construct an Expression with a single Number atom with value
    explicit Expression(double num);

    // Construct an Expression with a single Symbol atom with value
    explicit Expression(const std::string &sym);

    // Construct an Expression with a single Point atom with value
    explicit Expression(const std::tuple<double, double> &value);

    // Construct an Expression with a single Line atom with starting
    // point start and ending point end
    Expression(std::tuple<double, double> start,
               std::tuple<double, double> end);

    // Construct an Expression with a single Arc atom with center
    // point center, starting point start, and spanning angle angle in radians
    Expression(std::tuple<double, double> center,
               std::tuple<double, double> start,
               double angle);

    // Helpers
    explicit Expression(Atom atom) : head(std::move(atom)) {
    }

    explicit Expression(const Point &point);

    explicit Expression(const Line &line);

    explicit Expression(const Arc &arc);

    explicit Expression(const Rect &rect);

    explicit Expression(const FillRect &fillRect);

    explicit Expression(const Ellipse &ellipse);

    bool isList() const noexcept { return !tail.empty(); }
    Type headType() const noexcept { return head.type; }
    Value headValue() const noexcept { return head.value; }
    Atom &getHead() noexcept { return head; }
    const Atom &getHead() const noexcept { return head; }
    bool tailIsEmpty() const noexcept { return tail.empty(); }
    size_t tailSize() const noexcept { return tail.size(); }
    std::vector<Expression> &getTail() noexcept { return tail; }
    const std::vector<Expression> &getTail() const noexcept { return tail; }

    bool operator==(const Expression &exp) const noexcept;

    friend std::ostream &operator<<(std::ostream &out, const Expression &e);

    Atom head;
    std::vector<Expression> tail;
};


// A Procedure is a C++ function pointer taking
// a vector of Atoms as arguments
typedef Expression (*Procedure)(const std::vector<Atom> &args);

// map a token to an Atom
bool token_to_atom(const std::string &token, Atom &atom);

#endif
