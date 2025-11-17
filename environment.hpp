#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

// system includes
#include <unordered_map>
#include <utility>

// module includes
#include "expression.hpp"

class Environment {
public:
    Environment();

    void reset();

    void define(const Symbol &name, const Expression &value);

    bool is_symbol_bound(const Symbol &name) const;

    Expression get_symbol(const Symbol &name) const;

    bool is_procedure(const Symbol &name) const;

    bool is_reserved(const Symbol &name) const;

    Procedure get_procedure(const Symbol &name) const;

private:
    enum EnvResultType { ExpressionType, ProcedureType };

    struct EnvResult {
        EnvResultType type{};
        Expression exp;
        Procedure proc{};

        EnvResult() = default;

        EnvResult(const EnvResultType eType, Expression eExp) : type(eType), exp(std::move(eExp)) {
        }

        EnvResult(const EnvResultType eType, const Procedure eProc) : type(eType), exp(Expression()), proc(eProc) {
        }
    };

    std::unordered_map<Symbol, EnvResult> envmap;
};

#endif
