#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstring>

#include "Token.h"

class Variable;

static std::map<std::string, std::string (*)()> strFunc;
static std::map<std::string, Variable> variables;

static std::vector<Tok> tokens;
static std::vector<Tok> tokensOnLine;

const static std::vector<std::string> Funcs
{
    "PRINT",
    "LET",
    "IF",
    "ENDIF",
    "GOTO",
    "END"
};

static bool isDebug {false};

enum BoolOperator
{
    EQUALS,
    NOT,
    GREATER,
    LESS
};

enum Operator
{
    ADD,
    SUB,
    DIV,
    MUL,
    MOD
};

class Parser
{
public:
    Parser(bool);

    void Parse(std::vector<Tok>&);

private:
    void ParseLine();
    static void ParseID();

    std::vector<Tok>::iterator curToken;
    std::vector<Tok>::iterator endToken;
};

class Variable
{
public:
    enum TokenType dataType;
    std::string name, text;
    size_t lineNumber {1}, charIndex {1};
};

std::string Print();
std::string Let();
std::string If();
std::string EndIf();
std::string Goto();
std::string End();

void Goto(size_t);
void ReAssignVar();

signed long int Math();

bool CheckIf(const std::string&,
             enum BoolOperator,
             const std::string&);

bool InArray(const std::string &value, const std::vector<std::string> &array);
bool IsVar(const std::string &value);