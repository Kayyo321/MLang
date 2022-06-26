#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <fstream>
#include <cstring>

#include "Token.h"
#include "Lexer.h"

class Variable;
class Array;

static std::map<std::string, std::string (*)()> strFunc;
static std::map<std::string, Variable> variables;
static std::map<std::string, Array> arrays;
static std::map<std::string, std::vector<Tok>> portions;

static std::vector<Tok> tokens;
static std::vector<Tok> tokensOnLine;

static std::vector<std::string> scopeVars;

static std::vector<std::string> scopeArrs;

static std::vector<std::string> Funcs;

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
    explicit Parser(bool);

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
};

class Array
{
public:
    enum TokenType dataType;
    std::string name;

    std::vector<Tok> children;
};

std::string Print();
std::string Let();
std::string Arr();
std::string If();
std::string EndIf();
std::string Goto();
std::string End();
std::string Portion();
std::string Release();

void Goto(size_t);
void ReAssignVar();
void ReAssignArr();

signed long int Math();

bool CheckIf(const std::string&,
             enum BoolOperator,
             const std::string&);

bool InArray(const std::string &value, const std::vector<std::string> &array);
bool IsVar(const std::string &value);
bool IsArr(const std::string &value);
bool IsPort(const std::string &value);