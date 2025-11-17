#include "qt_interpreter.hpp"
#include "qgraphics_arc_item.hpp"
#include "interpreter_semantic_error.hpp"
#include "tokenizer.hpp"

#include <QPen>
#include <QGraphicsEllipseItem>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// TODO: simplify this module further as there is redundant checks / logic

static bool isGraphicalType(Type t) {
    return t == PointType || t == LineType || t == ArcType ||
           t == RectType || t == FillRectType || t == EllipseType;
}

// Remove ';' comments
static std::string stripLineComments(const std::string &src) {
    std::istringstream in(src);
    std::ostringstream out;
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        auto pos = line.find(';');
        if (pos != std::string::npos) line.erase(pos);
        // keep line endings
        if (!first) out << '\n';
        first = false;
        out << line;
    }
    return out.str();
}

static std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [](unsigned char ch) { return !std::isspace(ch); }));
    return s;
}

static std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

static std::string trim(std::string s) { return rtrim(ltrim(std::move(s))); }

// Balance checking parentheses, ignoring comments
static bool parensBalancedSkipComments(const std::string &s) {
    int depth = 0;
    bool in_comment = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (in_comment) {
            if (c == '\n') in_comment = false;
            continue;
        }
        if (c == ';') {
            in_comment = true;
            continue;
        }
        if (c == '(') ++depth;
        else if (c == ')') { if (--depth < 0) return false; }
    }
    return depth == 0;
}

static std::string &accumBuffer() {
    static std::string buf;
    return buf;
}

// Split tokens [lo, hi) into complete expressions
static std::vector<std::string>
splitArgsIntoExprs(const TokenSequenceType &toks, std::size_t lo, std::size_t hi) {
    std::vector<std::string> out;
    int depth = 0;
    std::string curr;

    auto flush = [&]() {
        if (!curr.empty() && depth == 0) {
            out.push_back(curr);
            curr.clear();
        }
    };

    for (std::size_t i = lo; i < hi; ++i) {
        const std::string &tk = toks[i];
        if (tk == "(") {
            if (!curr.empty()) curr.push_back(' ');
            curr.push_back('(');
            ++depth;
        } else if (tk == ")") {
            curr.push_back(')');
            --depth;
            if (depth == 0) flush();
        } else {
            if (!curr.empty() && curr.back() != '(') curr.push_back(' ');
            curr += tk;
            if (depth == 0) flush(); // single-atom argument TODO: fix this since this is illegal
        }
    }
    return out;
}

// Find every sub-expression of the form "( ... draw )" and return its [open, close] indices
static std::vector<std::pair<std::size_t, std::size_t> >
findDrawBlocks(const TokenSequenceType &toks) {
    std::vector<std::pair<std::size_t, std::size_t> > blocks;
    std::vector<std::size_t> stack;

    for (std::size_t i = 0; i < toks.size(); ++i) {
        if (toks[i] == "(") {
            stack.push_back(i);
        } else if (toks[i] == ")") {
            if (!stack.empty()) {
                std::size_t start = stack.back();
                stack.pop_back();
                if (i >= 2 && toks[i - 1] == "draw") {
                    blocks.emplace_back(start, i);
                }
            }
        }
    }
    return blocks;
}

QtInterpreter::QtInterpreter(QObject *parent) : QObject(parent), Interpreter() {
}

void QtInterpreter::flushPending() {
    // Access the accumulated text
    std::string &buf = accumBuffer();

    // if nothing to do, return
    {
        const std::string cleanedPeek = trim(stripLineComments(buf));
        if (cleanedPeek.empty()) {
            buf.clear();
            return;
        }
    }

    // At EOF, if unbalanced throw  parse error
    if (!parensBalancedSkipComments(buf)) {
        emit error(QString("Error: Invalid Expression. Could not parse (parentheses not balanced)."));
        buf.clear();
        return;
    }

    // balanced: strip comments and trim
    const std::string program = trim(stripLineComments(buf));
    buf.clear();
    if (program.empty()) return;

    // parse once
    std::istringstream iss(program);
    if (!parse(iss)) {
        emit error(QString("Error: Invalid Expression. Could not parse."));
        return;
    }

    // tokenize once to find nested ( ... draw ) blocks
    TokenSequenceType tokens;
    try {
        std::istringstream tss(program);
        tokens = tokenize(tss);
    } catch (...) {
        emit error(QString("Error: tokenization failed."));
        return;
    }

    try {
        // evaluate whole program
        Expression result = eval();

        // render every "( ... draw )" block anywhere in the program
        const auto blocks = findDrawBlocks(tokens);
        for (const auto &pr: blocks) {
            const std::size_t openIdx = pr.first;
            const std::size_t closeIdx = pr.second;
            if (closeIdx <= openIdx + 2) continue; // nothing inside

            const auto args = splitArgsIntoExprs(tokens, openIdx + 1, closeIdx - 1);
            for (const auto &argExpr: args) {
                const std::string cleanedArg = trim(stripLineComments(argExpr));
                if (cleanedArg.empty()) continue;

                Expression g = evaluateSubExpression(cleanedArg);
                if (isGraphicalType(g.headType())) {
                    createGraphicItem(g);
                }
            }
        }

        // emit result
        std::ostringstream oss;
        oss << result;
        emit info(QString::fromStdString(oss.str()));
    } catch (const InterpreterSemanticError &e) {
        emit error(QString("Error: ") + QString(e.what()));
    } catch (const std::exception &e) {
        emit error(QString("Error: ") + QString(e.what()));
    }
}


Expression QtInterpreter::evaluateSubExpression(const std::string &subExpr) {
    std::string cleaned = trim(stripLineComments(subExpr));
    if (cleaned.empty()) throw InterpreterSemanticError("Empty sub-expression");

    {
        std::istringstream tss(cleaned);
        TokenSequenceType t = tokenize(tss);
        if (t.empty()) throw InterpreterSemanticError("Empty sub-expression");
    }

    std::istringstream iss(cleaned);
    if (!parse(iss)) throw InterpreterSemanticError("Could not parse sub-expression");
    return eval();
}

void QtInterpreter::parseAndEvaluate(QString entry) {
    std::string &buf = accumBuffer();
    buf.append(entry.toStdString());
    buf.push_back('\n');

    {
        std::string tmp = trim(stripLineComments(buf));
        if (tmp.empty()) return;
    }


    if (!parensBalancedSkipComments(buf)) {
        return;
    }


    const std::string programRaw = buf;
    buf.clear();


    const std::string program = stripLineComments(programRaw);
    if (trim(program).empty()) {
        return;
    }

    // debug print
    std::cout << "Full program to parse and eval:\n" << program << "\n";

    // 1) Parse the full program once
    std::istringstream iss(program);
    if (!parse(iss)) {
        emit error(QString("Error: Invalid Expression. Could not parse."));
        return;
    }

    // 2) Tokenize once
    TokenSequenceType tokens;
    try {
        std::istringstream tss(program);
        tokens = tokenize(tss);
    } catch (...) {
        emit error(QString("Error: tokenization failed."));
        return;
    }

    try {
        // 3) Evaluate full program
        Expression result = eval();

        // 4) Render every nested block
        const auto blocks = findDrawBlocks(tokens);
        for (const auto &pr: blocks) {
            const std::size_t openIdx = pr.first;
            const std::size_t closeIdx = pr.second;
            if (closeIdx <= openIdx + 2) continue; // nothing inside

            const auto args = splitArgsIntoExprs(tokens, openIdx + 1, closeIdx - 1);
            for (const auto &argExpr: args) {
                // Skip accidental empties defensively
                if (trim(stripLineComments(argExpr)).empty()) continue;
                Expression g = evaluateSubExpression(argExpr);
                if (isGraphicalType(g.headType())) {
                    createGraphicItem(g);
                }
            }
        }

        std::ostringstream oss;
        oss << result;
        emit info(QString::fromStdString(oss.str()));
    } catch (const InterpreterSemanticError &e) {
        emit error(QString("Error: ") + QString(e.what()));
    } catch (const std::exception &e) {
        emit error(QString("Error: ") + QString(e.what()));
    }
}

void QtInterpreter::createGraphicItem(const Expression &exp) {
    const Type type = exp.headType();

    if (type == PointType) {
        const Point p = exp.headValue().point_value;
        auto *item = new QGraphicsEllipseItem(p.x - 2, p.y - 2, 4, 4);
        item->setBrush(QBrush(Qt::black));
        item->setPen(Qt::NoPen);
        emit drawGraphic(item);
        return;
    }

    if (type == LineType) {
        const Line l = exp.headValue().line_value;
        auto *item = new QGraphicsLineItem(l.start.x, l.start.y, l.end.x, l.end.y);
        item->setPen(QPen(Qt::black));
        emit drawGraphic(item);
        return;
    }

    if (type == ArcType) {
        const Arc arc = exp.headValue().arc_value;

        const double dx = (arc.start.x - arc.center.x);
        const double dy = (arc.start.y - arc.center.y);
        const double radius = std::sqrt(dx * dx + dy * dy);

        const double startAngleRad = std::atan2(-dy, dx);

        const int startAngleQt = int(startAngleRad * 180.0 / M_PI * 16.0);
        const int spanAngleQt = int(arc.angle * 180.0 / M_PI * 16.0);

        const qreal x = arc.center.x - radius;
        const qreal y = arc.center.y - radius;
        const qreal w = 2 * radius;
        const qreal h = 2 * radius;

        auto *item = new QGraphicsArcItem(x, y, w, h);
        item->setStartAngle(startAngleQt);
        item->setSpanAngle(spanAngleQt);

        QPen pen(Qt::black);
        pen.setWidthF(1.0);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        item->setPen(pen);

        emit drawGraphic(item);
        return;
    }

    if (type == RectType) {
        const Rect r = exp.headValue().rect_value;
        const double x = std::min(r.point1.x, r.point2.x);
        const double y = std::min(r.point1.y, r.point2.y);
        const double w = std::abs(r.point2.x - r.point1.x);
        const double h = std::abs(r.point2.y - r.point1.y);

        auto *item = new QGraphicsRectItem(x, y, w, h);
        item->setPen(QPen(Qt::black));
        item->setBrush(Qt::NoBrush);
        emit drawGraphic(item);
        return;
    }

    if (type == FillRectType) {
        const FillRect fr = exp.headValue().fill_rect_value;
        const double x = std::min(fr.rect.point1.x, fr.rect.point2.x);
        const double y = std::min(fr.rect.point1.y, fr.rect.point2.y);
        const double w = std::abs(fr.rect.point2.x - fr.rect.point1.x);
        const double h = std::abs(fr.rect.point2.y - fr.rect.point1.y);

        auto *item = new QGraphicsRectItem(x, y, w, h);
        item->setPen(Qt::NoPen);
        item->setBrush(QBrush(QColor(int(fr.r), int(fr.g), int(fr.b))));
        emit drawGraphic(item);
        return;
    }

    if (type == EllipseType) {
        const Ellipse e = exp.headValue().ellipse_value;
        const double x = std::min(e.rect.point1.x, e.rect.point2.x);
        const double y = std::min(e.rect.point1.y, e.rect.point2.y);
        const double w = std::abs(e.rect.point2.x - e.rect.point1.x);
        const double h = std::abs(e.rect.point2.y - e.rect.point1.y);

        auto *item = new QGraphicsEllipseItem(x, y, w, h);
        item->setPen(QPen(Qt::black));
        item->setBrush(Qt::NoBrush);
        emit drawGraphic(item);
        return;
    }
}
