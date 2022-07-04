#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "Token.h"

class Lexer
{
public:
    [[nodiscard]] std::vector<Tok> Parse(const char *, bool);

private:
    Tok curToken;
    std::vector<Tok> tokens;

    bool isDebug;

    void NewToken();
};