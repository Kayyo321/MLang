#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "Token.h"

class Lexer
{
public:
    std::vector<Token> Parse(const std::string&);

private:
    Token curToken;
    std::vector<Token> tokens;

    void NewToken();
};