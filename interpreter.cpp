#include "interpreter.hpp"

// system includes
#include <stack>

// module includes
#include <sstream>
#include <iosfwd>
#include <iostream>

#include "tokenizer.hpp"
#include "expression.hpp"
#include "environment.hpp"
#include "interpreter_semantic_error.hpp"

// Helper: parse a single atom token into 'exp'.
// Success => set 'exp', advance 'it', return true. Failure => return false.
// No parentheses or semantic checks here.
bool Interpreter::parse_atom(TokenSequenceType::const_iterator &it,
                             const TokenSequenceType::const_iterator &end,
                             Expression &exp) {

    if (it == end) {
        return false;
    }
    const std::string &token = *it;

    if (token == "(" || token == ")") {
        return false;
    }

    Atom atom;
    if (!token_to_atom(token, atom)) {
        return false;
    }

    exp = Expression(atom);
    ++it;
    return true;
}


// Parse one full expression (atom or parenthesized list) into 'exp'.
// Lists must satisfy postfix form: "( <expr> ... <expr> <symbol> )".
// Advance 'it' over the parsed expression. Return false on ANY syntax error.
bool Interpreter::parse_expression(TokenSequenceType::const_iterator &it,
                                   TokenSequenceType::const_iterator &end,
                                   Expression &exp) {
    if (it == end) {
        return false;
    }

    // Case 1: parenthesized form
    if (*it == "(") {
        ++it; // consume '('

        // collect sub-expressions until ')'
        std::vector<Expression> items;
        while (it != end && *it != ")") {
            Expression sub;
            if (!parse_expression(it, end, sub)) {
                return false; // nested parse failed
            }
            items.push_back(sub);
        }
        if (it == end || *it != ")") return false;
        ++it; // hit the end or consume ')'

        // Empty "()" is invalid
        if (items.empty()) {
            return false;
        }

        // Single element "(e)" → just grouping; becomes that expression directly.
        if (items.size() == 1) {
            exp = items[0];
            return true;
        }

        // Otherwise, postfix list: last must be a Symbol (operator / special form)
        const Expression &last = items.back();
        if (last.headType() != SymbolType || !last.tailIsEmpty()) {
            // last must be a plain symbol atom
            return false;
        }

        exp = Expression(last.headValue().sym_value); // head is the symbol
        exp.getTail().assign(items.begin(), items.end() - 1); // reset tail to all but last
        return true;
    }

    // Case 2: a single atom
    return parse_atom(it, end, exp);
}


// Simple utility: count how many tokens equal 'query' (e.g., "(", ")").
// Useful as a quick pre-check for paren balance before deep parsing.
std::size_t count(const TokenSequenceType &tokens, const std::string &query) {
    std::size_t cnt = 0;
    // (M3): iterate tokens and increment 'cnt' whenever token == query.
    for (const auto &t: tokens) {
        if (t == query) {
            ++cnt;
        }
    }
    return cnt;
}


// Entry point: tokenize input and build internal AST (this->ast).
// Return true on success; false on syntax errors. Do not throw here.
// Ensure there is **exactly one** top-level expression (no 0 or >1).
bool Interpreter::parse(std::istream &expression) noexcept {

    try {
        const TokenSequenceType tokens = tokenize(expression);

        if (tokens.empty()) {
            return false; // no tokens
        }

        // TODO: fix failing test
        // A single token is valid only if it's a Number, Boolean, or None
        // if (tokens.size() == 1) {
        //     const std::string &t = tokens.front();
        //     if (t != "(" && t != ")") {
        //         Atom a;
        //         if (token_to_atom(t, a)) {
        //             if (a.type == SymbolType) {
        //                 // lone symbol is invalid
        //                 return false;
        //             }
        //             // numbers & booleans & none are valid
        //         } else {
        //             // not even a valid atom
        //             return false;
        //         }
        //     }
        // }

        if (count(tokens, "(") != count(tokens, ")")) {
            return false; // unbalanced parens
        }

        auto it = tokens.cbegin();
        auto end = tokens.cend();
        Expression root;
        if (!parse_expression(it, end, root)) {
            return false; // parse failed
        }

        if (it != end) {
            return false; // extra tokens after single expr
        }

        ast = root;
        debug();
        return true;
    } catch (...) {
        return false; // catch-all for any unexpected errors
    }
}

bool is_graphic_atom(const Expression &e) {
    return e.tailIsEmpty() && (
               e.headType() == PointType ||
               e.headType() == LineType ||
               e.headType() == ArcType ||
               e.headType() == RectType ||
               e.headType() == FillRectType ||
               e.headType() == EllipseType
           );
}

// Evaluate an expression in 'env' and return a single-atom result.
// Throw InterpreterSemanticError on semantic errors.
// Atom: Symbol→lookup (throw if unknown); Number/Boolean/None→as-is.
// List: eval args (all but last), then apply LAST as special form or procedure.
Expression Interpreter::eval(const Expression &exp) {
    // case 1: atom (no tail)
    if (exp.tailIsEmpty()) {
        switch (exp.headType()) {
            case NoneType:
            case NumberType:
            case BooleanType:
                return exp; // literal
            case SymbolType: {
                if (!env.is_symbol_bound(exp.headValue().sym_value)) {
                    throw InterpreterSemanticError("Undefined symbol: " + exp.headValue().sym_value);
                }
                return env.get_symbol(exp.headValue().sym_value);
            }
            case PointType:
            case LineType:
            case ArcType:
            case RectType:
            case FillRectType:
            case EllipseType:
                return exp;
            default:
                throw InterpreterSemanticError("eval: default case reached unexpectedly");
        }
    }

    // case 2: list (non-empty tail)
    // The head must be a Symbol (operator or special form)
    if (exp.headType() != SymbolType) {
        throw InterpreterSemanticError("Malformed expression: non-symbol head in list");
    }

    const std::string &op = exp.headValue().sym_value;

    // case 3.1: check for special forms
    if (op == "define") {
        // (symbol expr define)
        if (exp.tailSize() != 2) {
            throw InterpreterSemanticError("define: wrong number of arguments");
        }
        const Expression &symExp = exp.getTail()[0];
        if (!(symExp.tailIsEmpty() && symExp.headType() == SymbolType)) {
            throw InterpreterSemanticError("define: first argument must be a symbol");
        }
        if (env.is_reserved(symExp.headValue().sym_value)) {
            throw InterpreterSemanticError("define: cannot redefine built-in symbol: " + symExp.headValue().sym_value);
        }
        Expression value = eval(exp.getTail()[1]); // evaluate the value expr
        env.define(symExp.headValue().sym_value, value);
        return value;
    }

    if (op == "begin") {
        // (e1 e2 ... begin) → evaluate in order, return last
        if (exp.tailIsEmpty()) {
            throw InterpreterSemanticError("begin: requires at least one expression");
        }
        Expression last;
        for (const auto &child: exp.getTail()) {
            last = eval(child);
        }
        return last;
    }

    if (op == "if") {
        // (cond then-expr else-expr if)
        if (exp.tailSize() != 3) {
            throw InterpreterSemanticError("if: wrong number of arguments");
        }
        Expression cond = eval(exp.getTail()[0]);
        if (!(cond.tailIsEmpty() && cond.headType() == BooleanType)) {
            throw InterpreterSemanticError("if: condition must be Boolean");
        }
        if (cond.headValue().bool_value) {
            return eval(exp.getTail()[1]);
        }

        return eval(exp.getTail()[2]);
    }

    if (op == "draw") {
        for (std::size_t i = 0; i < exp.getTail().size(); ++i) {
            Expression v = eval(exp.getTail()[i]);
            if (!is_graphic_atom(v)) {
                std::ostringstream oss;
                oss << "draw: argument " << (i + 1) << " is not a graphical object";
                throw InterpreterSemanticError(oss.str());
            }
        }
        return Expression();
    }

    // case 3.2: Regular Procedures
    // Evaluate all arguments left -> right (no short-circuit)
    std::vector<Atom> args;
    args.reserve(exp.tailSize());
    for (const auto &child: exp.getTail()) {
        Expression v = eval(child);
        args.push_back(v.getHead());
    }

    // look up procedure by name (throw if unknown)
    if (!env.is_procedure(op)) {
        throw InterpreterSemanticError("Unknown procedure: " + op);
    }
    Procedure proc = env.get_procedure(op);

    // apply procedure: returns expression atom or throws
    Expression result = proc(args);
    return result;
}


// Evaluate the AST previously produced by parse(). May update env (e.g., define).
// On any semantic error, throw InterpreterSemanticError.
Expression Interpreter::eval() {
    Expression result;
    return eval(ast);
}

// Optional: print/dump internal state for debugging (keep silent for grading).
void Interpreter::debug() const {
#ifdef POSTLISP_DEBUG_AST

#endif
}
