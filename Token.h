#pragma once

#include <string>
#include <iostream>

enum TokenType
{
    WHITESPACE,
    IDENTIFIER,
    OPERATOR,
    INT,
    MAYBE_DOUBLE,
    DOUBLE,
    STRING,
    SES,
    MAYBE_COMMENT,
    COMMENT,
    EOL,
    NULL_VAL
};

const static char *TokenTypeStrings[]
{
    "WHITESPACE",
    "IDENTIFIER",
    "OPERATOR",
    "INT",
    "MAYBE_DOUBLE",
    "DOUBLE",
    "STRING",
    "SES",
    "MAYBE_COMMENT",
    "COMMENT",
    "EOL"
};

class Tok
{
public:
    enum TokenType type;
    std::string text;
    size_t lineNumber {10}, charIndex {1};
};