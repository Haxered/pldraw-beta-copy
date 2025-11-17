#include "tokenizer.hpp"
#include <cctype>
#include <istream>

void store_ifnot_empty(std::string &token, TokenSequenceType &seq) {
    if (!token.empty()) {
        seq.push_back(token);
        token.clear();
    }
}

TokenSequenceType tokenize(std::istream &seq) {
    TokenSequenceType tokens;
    std::string cur;

    int ci;
    while ((ci = seq.get()) != EOF) {
        char ch = static_cast<char>(ci);

        // comments: ';' to end of line (consume newline, too)
        if (ch == ';') {
            store_ifnot_empty(cur, tokens);
            int cj;
            while ((cj = seq.get()) != EOF) {
                char c2 = static_cast<char>(cj);
                if (c2 == '\n') {
                    break;
                }
            }
            continue;
        }

        // parens are standalone tokens
        if (ch == '(' || ch == ')') {
            store_ifnot_empty(cur, tokens);
            tokens.emplace_back(1, ch); // "(" or ")"
            continue;
        }

        // whitespace: flush current token
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            store_ifnot_empty(cur, tokens);
            continue;
        }

        // otherwise, accumulate into current token
        cur.push_back(ch);
    }

    // 5) Flush any final token at EOF
    store_ifnot_empty(cur, tokens);
    return tokens;
}
