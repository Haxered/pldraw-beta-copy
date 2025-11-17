#include "environment.hpp"
#include "interpreter_semantic_error.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

static bool is_number(const Atom &a) { return a.type == NumberType; }
static bool is_boolean(const Atom &a) { return a.type == BooleanType; }

static double as_number(const Atom &a, const std::string &op) {
    if (!is_number(a)) {
        throw InterpreterSemanticError(op + ": argument must be Number");
    }
    return a.value.num_value;
}

static bool as_bool(const Atom &a, const std::string &op) {
    if (!is_boolean(a)) {
        throw InterpreterSemanticError(op + ": argument must be Boolean");
    }
    return a.value.bool_value;
}

static Point as_point(const Atom &a, const std::string &op) {
    if (a.type != PointType) {
        throw InterpreterSemanticError(op + ": argument must be Point");
    }
    return a.value.point_value;
}

// Extract Rect from Atom
static Rect as_rect(const Atom &a, const std::string &op) {
    if (a.type != RectType) {
        throw InterpreterSemanticError(op + ": argument must be Rect");
    }
    return a.value.rect_value;
}

static Expression make_num(double v) { return Expression(v); }
static Expression make_bool(bool v) { return Expression(v); }

// Equality tolerance for numbers
static bool num_eq(double a, double b) {
    constexpr double eps = std::numeric_limits<double>::epsilon();
    return std::fabs(a - b) <= eps;
}

// Built-in procedures
static Expression proc_add(const std::vector<Atom> &args) {
    if (args.empty()) {
        throw InterpreterSemanticError("+: requires at least one argument");
    }
    double s = 0.0;
    for (const auto &a: args) {
        s += as_number(a, "+");
    }
    return make_num(s);
}

static Expression proc_mul(const std::vector<Atom> &args) {
    if (args.empty()) {
        throw InterpreterSemanticError("*: requires at least one argument");
    }
    double p = 1.0;
    for (const auto &a: args) {
        p *= as_number(a, "*");
    }
    return make_num(p);
}

static Expression proc_sub(const std::vector<Atom> &args) {
    const std::string op = "-";
    if (args.size() == 1) {
        return make_num(-as_number(args[0], op)); // unary negation
    }
    if (args.size() == 2) {
        return make_num(as_number(args[0], op) - as_number(args[1], op));
    }
    throw InterpreterSemanticError("-: wrong number of arguments");
}

static Expression proc_div(const std::vector<Atom> &args) {
    const std::string op = "/";
    if (args.size() != 2) {
        throw InterpreterSemanticError("/: wrong number of arguments");
    }
    double a = as_number(args[0], op);
    double b = as_number(args[1], op);
    if (b == 0.0) {
        throw InterpreterSemanticError("/: division by zero");
    }
    return make_num(a / b);
}

static Expression proc_not(const std::vector<Atom> &args) {
    const std::string op = "not";
    if (args.size() != 1) {
        throw InterpreterSemanticError("not: wrong number of arguments");
    }
    return make_bool(!as_bool(args[0], op));
}

static Expression proc_and(const std::vector<Atom> &args) {
    const std::string op = "and";
    if (args.empty()) {
        throw InterpreterSemanticError("and: requires at least one argument");
    }
    bool acc = true;
    for (const auto &a: args) {
        acc = acc && as_bool(a, op);
    }
    return make_bool(acc);
}

static Expression proc_or(const std::vector<Atom> &args) {
    const std::string op = "or";
    if (args.empty()) {
        throw InterpreterSemanticError("or: requires at least one argument");
    }
    bool acc = false;
    for (const auto &a: args) {
        acc = acc || as_bool(a, op);
    }
    return make_bool(acc);
}

static Expression proc_lt(const std::vector<Atom> &args) {
    const std::string op = "<";
    if (args.size() != 2) {
        throw InterpreterSemanticError("<: wrong number of arguments");
    }
    return make_bool(as_number(args[0], op) < as_number(args[1], op));
}

static Expression proc_le(const std::vector<Atom> &args) {
    const std::string op = "<=";
    if (args.size() != 2) {
        throw InterpreterSemanticError("<=: wrong number of arguments");
    }
    return make_bool(as_number(args[0], op) <= as_number(args[1], op));
}

static Expression proc_gt(const std::vector<Atom> &args) {
    const std::string op = ">";
    if (args.size() != 2) {
        throw InterpreterSemanticError(">: wrong number of arguments");
    }
    return make_bool(as_number(args[0], op) > as_number(args[1], op));
}

static Expression proc_ge(const std::vector<Atom> &args) {
    const std::string op = ">=";
    if (args.size() != 2) {
        throw InterpreterSemanticError(">=: wrong number of arguments");
    }
    return make_bool(as_number(args[0], op) >= as_number(args[1], op));
}

static Expression proc_eq(const std::vector<Atom> &args) {
    const std::string op = "==";
    if (args.size() != 2) {
        throw InterpreterSemanticError("==: wrong number of arguments");
    }
    return make_bool(num_eq(as_number(args[0], op), as_number(args[1], op)));
}

static Expression proc_sqrt(const std::vector<Atom> &args) {
    const std::string op = "sqrt";
    if (args.size() != 1) {
        throw InterpreterSemanticError("sqrt: wrong number of arguments");
    }
    double x = as_number(args[0], op);
    if (x < 0.0) {
        throw InterpreterSemanticError("sqrt: domain error");
    }
    return make_num(std::sqrt(x));
}

static Expression proc_log2(const std::vector<Atom> &args) {
    const std::string op = "log2";
    if (args.size() != 1) {
        throw InterpreterSemanticError("log2: wrong number of arguments");
    }
    double x = as_number(args[0], op);
    if (x <= 0.0) {
        throw InterpreterSemanticError("log2: domain error");
    }
    return make_num(std::log2(x));
}

static Expression proc_sin(const std::vector<Atom> &args) {
    if (args.size() != 1) {
        throw InterpreterSemanticError("sin: wrong number of arguments");
    }
    return make_num(std::sin(as_number(args[0], "sin")));
}

static Expression proc_cos(const std::vector<Atom> &args) {
    if (args.size() != 1) {
        throw InterpreterSemanticError("cos: wrong number of arguments");
    }
    return make_num(std::cos(as_number(args[0], "cos")));
}

static Expression proc_arctan(const std::vector<Atom> &args) {
    if (args.size() != 2) {
        throw InterpreterSemanticError("arctan: wrong number of arguments");
    }
    double y = as_number(args[0], "arctan");
    double x = as_number(args[1], "arctan");
    return make_num(std::atan2(y, x));
}

// geometry
static double n(const Atom &a, const char *op) {
    return as_number(a, op);
}

static Expression proc_point(const std::vector<Atom> &args) {
    if (args.size() != 2) {
        throw InterpreterSemanticError("point: wrong number of arguments");
    }
    return Expression(Point{n(args[0], "point"), n(args[1], "point")});
}

static Expression proc_line(const std::vector<Atom> &args) {
    if (args.size() != 2) {
        throw InterpreterSemanticError("line: wrong number of arguments");
    }
    Point start = as_point(args[0], "line");
    Point end = as_point(args[1], "line");
    return Expression(Line{start, end});
}

static Expression proc_arc(const std::vector<Atom> &args) {
    if (args.size() != 3) {
        throw InterpreterSemanticError("arc: wrong number of arguments");
    }
    Point center = as_point(args[0], "arc");
    Point start = as_point(args[1], "arc");
    double angle = as_number(args[2], "arc");
    return Expression(Arc{center, start, angle});
}

static Expression proc_rect(const std::vector<Atom> &args) {
    if (args.size() != 2) {
        throw InterpreterSemanticError("rect: wrong number of arguments");
    }
    Point p1 = as_point(args[0], "rect");
    Point p2 = as_point(args[1], "rect");
    return Expression(Rect{p1, p2});
}

static Expression proc_fill_rect(const std::vector<Atom> &args) {
    if (args.size() != 4) {
        throw InterpreterSemanticError("fill_rect: wrong number of arguments");
    }
    Rect r = as_rect(args[0], "fill_rect");
    double red = as_number(args[1], "fill_rect");
    double green = as_number(args[2], "fill_rect");
    double blue = as_number(args[3], "fill_rect");
    return Expression(FillRect{r, red, green, blue});
}

static Expression proc_ellipse(const std::vector<Atom> &args) {
    if (args.size() != 1) {
        throw InterpreterSemanticError("ellipse: wrong number of arguments");
    }
    Rect r = as_rect(args[0], "ellipse");
    return Expression(Ellipse{r});
}

Environment::Environment() { reset(); }

void Environment::reset() {
    envmap.clear();

    // constants
    const double pi = std::atan2(0.0, -1.0);
    envmap.emplace(Symbol("pi"), EnvResult(ExpressionType, Expression(pi)));

    // arithmetic
    envmap.emplace(Symbol("+"), EnvResult(ProcedureType, &proc_add));
    envmap.emplace(Symbol("-"), EnvResult(ProcedureType, &proc_sub));
    envmap.emplace(Symbol("*"), EnvResult(ProcedureType, &proc_mul));
    envmap.emplace(Symbol("/"), EnvResult(ProcedureType, &proc_div));

    // logic
    envmap.emplace(Symbol("not"), EnvResult(ProcedureType, &proc_not));
    envmap.emplace(Symbol("and"), EnvResult(ProcedureType, &proc_and));
    envmap.emplace(Symbol("or"), EnvResult(ProcedureType, &proc_or));

    // comparison
    envmap.emplace(Symbol("<"), EnvResult(ProcedureType, &proc_lt));
    envmap.emplace(Symbol("<="), EnvResult(ProcedureType, &proc_le));
    envmap.emplace(Symbol(">"), EnvResult(ProcedureType, &proc_gt));
    envmap.emplace(Symbol(">="), EnvResult(ProcedureType, &proc_ge));
    envmap.emplace(Symbol("=="), EnvResult(ProcedureType, &proc_eq));

    // math
    envmap.emplace(Symbol("sqrt"), EnvResult(ProcedureType, &proc_sqrt));
    envmap.emplace(Symbol("log2"), EnvResult(ProcedureType, &proc_log2));
    envmap.emplace(Symbol("sin"), EnvResult(ProcedureType, &proc_sin));
    envmap.emplace(Symbol("cos"), EnvResult(ProcedureType, &proc_cos));
    envmap.emplace(Symbol("arctan"), EnvResult(ProcedureType, &proc_arctan));

    // geometry
    envmap.emplace(Symbol("point"), EnvResult(ProcedureType, &proc_point));
    envmap.emplace(Symbol("line"), EnvResult(ProcedureType, &proc_line));
    envmap.emplace(Symbol("arc"), EnvResult(ProcedureType, &proc_arc));
    envmap.emplace(Symbol("rect"), EnvResult(ProcedureType, &proc_rect));
    envmap.emplace(Symbol("fill_rect"), EnvResult(ProcedureType, &proc_fill_rect));
    envmap.emplace(Symbol("ellipse"), EnvResult(ProcedureType, &proc_ellipse));

    envmap.emplace(Symbol("define"), EnvResult(ProcedureType, nullptr));
    envmap.emplace(Symbol("begin"), EnvResult(ProcedureType, nullptr));
    envmap.emplace(Symbol("if"), EnvResult(ProcedureType, nullptr));
    envmap.emplace(Symbol("draw"), EnvResult(ProcedureType, nullptr));
}

// Define or rebind a symbol to a concrete Expression value
void Environment::define(const Symbol &name, const Expression &value) {
    envmap[name] = EnvResult(ExpressionType, value);
}

// Is there a bound value with this name?
bool Environment::is_symbol_bound(const Symbol &name) const {
    auto it = envmap.find(name);
    return it != envmap.end() && it->second.type == ExpressionType;
}

// Get the bound value (throws if missing or not a value)
Expression Environment::get_symbol(const Symbol &name) const {
    auto it = envmap.find(name);
    if (it == envmap.end() || it->second.type != ExpressionType) {
        throw InterpreterSemanticError("Unbound symbol: " + name);
    }
    return it->second.exp;
}

// Is there a procedure with this name?
bool Environment::is_procedure(const Symbol &name) const {
    auto it = envmap.find(name);
    return it != envmap.end() && it->second.type == ProcedureType && it->second.proc != nullptr;
}

// User defined variables can not be overriden according to reference binary
// Is this a reserved symbol / keyword (cannot be redefined)?
bool Environment::is_reserved(const Symbol &name) const {
    auto it = envmap.find(name);
    return it != envmap.end();
}

// Get the procedure pointer (throws if missing or not a procedure)
Procedure Environment::get_procedure(const Symbol &name) const {
    auto it = envmap.find(name);
    if (it == envmap.end() || it->second.type != ProcedureType || it->second.proc == nullptr) {
        throw InterpreterSemanticError("Unknown procedure: " + name);
    }
    return it->second.proc;
}
