#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "expression.hpp"
#include "environment.hpp"
#include "tokenizer.hpp"

// Interpreter has
// Environment, which starts at a default
// parse method, builds an internal AST
// eval method, updates Environment, returns last result
class Interpreter {
public:
    bool parse(std::istream &expression) noexcept;

    Expression eval();

private:
    Expression eval(const Expression &exp);

    static bool parse_atom(TokenSequenceType::const_iterator &it, const TokenSequenceType::const_iterator &end,
                           Expression &exp);

    bool parse_expression(TokenSequenceType::const_iterator &it, TokenSequenceType::const_iterator &end,
                          Expression &exp);

    Environment env;
    Expression ast;

    void debug() const;
};


#endif
