#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <deque>
#include <string>

typedef std::deque<std::string> TokenSequenceType;

// split string into a list of tokens where a token is one of
// OPEN "(" or CLOSE ")" or a space-delimited string
// ignores any whitespace and from any ";" to end-of-line
TokenSequenceType tokenize(std::istream &seq);

#endif
