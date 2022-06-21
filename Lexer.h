#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "Token.h"

class Lexer
{
public:
    std::vector<Tok> Parse(const std::string&, bool);

private:
    Tok curToken;
    std::vector<Tok> tokens;

    bool isDebug;

    void NewToken();
};