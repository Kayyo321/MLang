#include "Parser.h"

bool isGoto {false};
bool isIf {false};

Parser::Parser(bool _isDebug)
{
    isDebug = _isDebug;

    strFunc["PRINT"] = &Print;
    strFunc["LET"] = &Let;
    strFunc["ARR"] = &Arr;
    strFunc["IF"] = &If;
    strFunc["ENDIF"] = &EndIf;
    strFunc["GOTO"] = &Goto;
    strFunc["END"] = &End;
    strFunc["PORTION"] = &Portion;
    strFunc["RELEASE"] = &Release;
    strFunc["APPEND"] = &Append;
    strFunc["STR_COMB"] = &StrComb;
    strFunc["FREE"] = &Free;
    strFunc["FREE_ARR"] = &FreeArr;
    strFunc["FREE_PORTION"] = &FreePort;

    Funcs.emplace_back("PRINT");
    Funcs.emplace_back("LET");
    Funcs.emplace_back("ARR");
    Funcs.emplace_back("IF");
    Funcs.emplace_back("ENDIF");
    Funcs.emplace_back("GOTO");
    Funcs.emplace_back("END");
    Funcs.emplace_back("PORTION");
    Funcs.emplace_back("RELEASE");
    Funcs.emplace_back("APPEND");
    Funcs.emplace_back("STR_COMB");
    Funcs.emplace_back("FREE");
    Funcs.emplace_back("FREE_ARR");
    Funcs.emplace_back("FREE_PORTION");
}

std::vector<Tok>::iterator setToken;

void Parser::Parse(const std::vector<Tok> &_tokens)
{
    tokens = _tokens;

    curToken = tokens.begin();
    endToken = tokens.end();

    while (curToken != endToken)
    {
        tokensOnLine.clear();

        setToken = curToken;

        ParseLine();

        if (isGoto)
        {
            isGoto = false;
            curToken = setToken;
        }

        if (curToken == endToken)
            break;
    }

    if (isDebug)
    {
        std::cout << "DEBUG FINISHING OUTPUT\n\n";

        for (const auto &pair: variables)
        {
            std::cout
                << pair.first
                << ", has the value of: "
                << pair.second.text
                << "\n"
                << pair.first
                << ", has the data-type of: "
                << TokenTypeStrings[pair.second.dataType]
                << "\n\n";
        }

        for (const auto &pair: arrays)
        {
            std::cout
                << "Array: \""
                << pair.first
                << "\": {\n";

            for (const Tok &token: pair.second.children)
            {
                std::cout
                    << "\t"
                    << token.text
                    << "\n";
            }

            std::cout << "}\n\n";
        }

        for (const auto &pair: portions)
        {
            std::cout << "Portion: " << pair.first << "\n";
        }

        std::cout << "\nMLANG DEBUG ENDING...";
    }
}

void Parser::ParseLine()
{
    if (curToken->type == EOL && curToken != endToken)
        ++curToken;

    while (curToken->type != EOL && curToken != endToken)
    {
        tokensOnLine.push_back(*curToken);

        ++curToken;
    }

    ParseID();
}

void Parser::ParseID()
{
    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &tok {tokensOnLine[i]};

        if (tok.type == IDENTIFIER)
        {
            if (InArray(tok.text, Funcs))
            {
                /* Function call */

                strFunc[tok.text]();

                break;
            }

            if (IsVar(tok.text))
            {
                if (tokensOnLine[i + 1].type == OPERATOR
                    && tokensOnLine[i + 1].text == ":")
                {
                    ReAssignVar();

                    break;
                }
                else if (tokensOnLine[i + 1].type == OPERATOR
                         && tokensOnLine[i + 1].text != ":")
                {
                    if (tokensOnLine[i + 2].type != STRING)
                    {
                        throw std::runtime_error
                        (
                            std::string("Unexpected data-type: ")
                            + tokensOnLine[i].text
                            + std::string(". TE_MATH needs a string (")
                            + std::to_string(tokensOnLine[i].lineNumber)
                            + std::string(", ")
                            + std::to_string(tokensOnLine[i].charIndex)
                            + std::string(").")
                        );
                    }

                    const char *expr {tokensOnLine[i + 2].text.c_str()};
                    const char *varName {tokensOnLine[0].text.c_str()};
                    const long double newVal {Math(expr)};

                    variables[varName].text = std::to_string(newVal);
                }
                break;
            }

            if (IsArr(tok.text))
            {
                ReAssignArr();

                break;
            }
        }
    }
}

void Print()
{
    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        if (i == 0) continue;

        const Tok &curToken {tokensOnLine[i]};

        if (curToken.type == IDENTIFIER)
        {
            if (InArray(curToken.text, Funcs))
            {
                /* Function call */

                strFunc[curToken.text]();

                continue;
            }

            if (IsVar(curToken.text))
            {
                std::cout << variables[curToken.text].text;

                continue;
            }

            if (IsArr(curToken.text))
            {
                if (tokensOnLine[i + 1].type == INT)
                {
                    const int index {std::stoi(tokensOnLine[i + 1].text)};

                    std::cout << arrays[curToken.text].children[index].text;

                    ++i;
                }
                else if (IsVar(tokensOnLine[i + 1].text))
                {
                    if (variables[tokensOnLine[i + 1].text].dataType == INT)
                    {
                        const int index {std::stoi(variables[tokensOnLine[i + 1].text].text)};

                        std::cout << arrays[curToken.text].children[index].text;

                        ++i;
                    }
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Index should be integer: ")
                        + tokensOnLine[i + 1].text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }

                continue;
            }

            throw std::runtime_error
            (
                std::string("Unknown identifier: ")
                + curToken.text
                + std::string(". (")
                + std::to_string(curToken.lineNumber)
                + std::string(", ")
                + std::to_string(curToken.charIndex)
                + std::string(").")
            );
        }
        else
        {
            std::cout << curToken.text;
        }
    }
}

void Let()
{
    Variable x;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (curToken.type == IDENTIFIER)
                {
                    if (IsVar(tokensOnLine[1].text))
                    {
                        throw std::runtime_error
                        (
                            std::string("Variable name already exists: ")
                            + curToken.text
                            + std::string(". (")
                            + std::to_string(curToken.lineNumber)
                            + std::string(", ")
                            + std::to_string(curToken.charIndex)
                            + std::string(").")
                        );
                    }

                    const char *_name {curToken.text.c_str()};

                    if (curToken.text[0] == '$')
                    {
                        x.isConst = true;

                        _name = "";

                        for (size_t o {0}; o < curToken.text.size(); ++o)
                        {
                            if (o != 0)
                                _name += curToken.text[o];
                        }
                    }

                    x.name = _name;
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected variable name: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 2:
                if (curToken.type == OPERATOR
                    && curToken.text == ":")
                {
                    continue;
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Unknown operator: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }

            case 3:
            {
                if (curToken.type == IDENTIFIER)
                {
                    if (curToken.text == "TE_MATH")
                    {
                        const char *_tokens {tokensOnLine[4].text.c_str()};
                        _tokens = GetVarsInStr(_tokens).c_str();
                        long double val {Math(_tokens)};

                        if (IsWhole(val))
                        {
                            x.dataType = INT;
                            x.text = std::to_string((int)val);
                        }
                        else
                        {
                            x.dataType = DOUBLE;
                            x.text = std::to_string((double)val);
                        }

                        ++i;
                    }
                    else if (IsVar(curToken.text))
                    {
                        x.dataType = variables[curToken.text].dataType;
                        x.text = variables[curToken.text].text;
                    }
                    else
                    {
                        throw std::runtime_error
                        (
                            std::string("Unexpected identifier: ")
                            + curToken.text
                            + std::string(". (")
                            + std::to_string(curToken.lineNumber)
                            + std::string(", ")
                            + std::to_string(curToken.charIndex)
                            + std::string(").")
                        );
                    }
                }
                else
                {
                    x.dataType = curToken.type;
                    x.text = curToken.text;
                }
            }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
        }
    }

    variables[x.name] = x;

    if (isIf)
        scopeVars.emplace_back(x.name);
}

void Arr()
{
    Array x;
    x.dataType = NULL_VAL;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &token {tokensOnLine[i]};

        if (i == 0)
            continue;

        if (i == 1)
        {
            if (token.type == IDENTIFIER)
            {
                if (IsArr(token.text))
                {
                    throw std::runtime_error
                    (
                        std::string("Array name already exists: ")
                        + token.text
                        + std::string(". (")
                        + std::to_string(token.lineNumber)
                        + std::string(", ")
                        + std::to_string(token.charIndex)
                        + std::string(").")
                    );
                }

                x.name = token.text;

                continue;
            }
            else
            {
                throw std::runtime_error
                (
                    std::string("Unknown variable name: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );
            }
        }

        if (i == 2)
        {
            if (token.type == OPERATOR
                && token.text == ":")
            {
                continue;
            }
            else
            {
                throw std::runtime_error
                (
                    std::string("Unknown operator: ")
                    + token.text
                    + std::string(". Expected ':' (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );
            }
        }

        if (i >= 3)
        {
            if (token.type != EOL)
            {
                if (x.dataType == NULL_VAL)
                {
                    if (token.type == IDENTIFIER)
                    {
                        if (IsVar(token.text))
                        {
                            x.dataType = variables[token.text].dataType;
                            x.children.push_back(token);

                            continue;
                        }

                        throw std::runtime_error
                        (
                            std::string("Unknown identifier: ")
                            + token.text
                            + std::string(". (")
                            + std::to_string(token.lineNumber)
                            + std::string(", ")
                            + std::to_string(token.charIndex)
                            + std::string(").")
                        );
                    }
                    else
                    {
                        x.dataType = token.type;
                        x.children.push_back(token);
                    }
                }
                else
                {
                    if (token.type == IDENTIFIER)
                    {
                        if (IsVar(token.text))
                        {
                            if (x.dataType != variables[token.text].dataType)
                            {
                                throw std::runtime_error
                                (
                                    std::string("Unexpected data-type: ")
                                    + token.text
                                    + std::string(". (")
                                    + std::to_string(token.lineNumber)
                                    + std::string(", ")
                                    + std::to_string(token.charIndex)
                                    + std::string(").")
                                );
                            }

                            x.children.push_back(token);

                            continue;
                        }

                        throw std::runtime_error
                        (
                            std::string("Unknown identifier: ")
                            + token.text
                            + std::string(". (")
                            + std::to_string(token.lineNumber)
                            + std::string(", ")
                            + std::to_string(token.charIndex)
                            + std::string(").")
                        );
                    }
                    else
                    {
                        if (x.dataType != token.type)
                        {
                            throw std::runtime_error
                            (
                                std::string("Unexpected data-type: (")
                                + token.text
                                + std::string(", ")
                                + TokenTypeStrings[token.type]
                                + std::string(") against: (")
                                + TokenTypeStrings[x.dataType]
                                + std::string("). (")
                                + std::to_string(token.lineNumber)
                                + std::string(", ")
                                + std::to_string(token.charIndex)
                                + std::string(").")
                            );
                        }

                        x.children.push_back(token);
                    }
                }
            }
        }
    }

    arrays[x.name] = x;

    if (isIf)
        scopeArrs.emplace_back(x.name);
}

void If()
{
    const char *lhs, *rhs;
    BoolOperator op;

    unsigned long long endif;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (curToken.type == IDENTIFIER)
                {
                    if (IsVar(curToken.text))
                    {
                        lhs = (variables[curToken.text].text.c_str());

                        break;
                    }

                    throw std::runtime_error
                    (
                        std::string("Unexpected Identifier: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                else
                {
                    lhs = curToken.text.c_str();
                }
                break;

            case 2:
                if (curToken.type != OPERATOR)
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected token: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }

                if (curToken.text == "?")
                    op = EQUALS;
                else if (curToken.text == "!")
                    op = NOT;
                else if (curToken.text == "<")
                    op = LESS;
                else if (curToken.text == ">")
                    op = GREATER;
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected operator: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 3:
                if (curToken.type == IDENTIFIER)
                {
                    if (IsVar(curToken.text))
                    {
                        rhs = (variables[curToken.text].text.c_str());

                        break;
                    }

                    throw std::runtime_error
                    (
                        std::string("Unexpected Identifier: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                else
                {
                    rhs = curToken.text.c_str();
                }
                break;

            case 4:
                if (tokensOnLine[4].type == INT)
                {
                    endif = stoi(tokensOnLine[4].text);
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("ENDIF needs an integer: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
        }
    }

    bool True {CheckIf(lhs, op, rhs)};

    if (!True)
    {
        Goto(endif);
        goto Skip;
    }

    isIf = true;

Skip:
    return;
}

void EndIf()
{
    isIf = false;

    if (!scopeVars.empty())
    {
        for (const std::string &s: scopeVars)
        {
            if (IsVar(s))
            {
                variables.erase(s);
            }
        }
    }

    if (!scopeArrs.empty())
    {
        for (const std::string &s: scopeArrs)
        {
            if (IsArr(s))
            {
                arrays.erase(s);
            }
        }
    }
}

void Goto(size_t line)
{
    bool foundLine {false};

    for (const Tok &tok: tokens)
    {
        if (line == tok.lineNumber)
        {
            foundLine = true;
            break;
        }
    }

    if (!foundLine)
    {
        throw std::runtime_error
        (
            std::string("GOTO out of range: ")
            + tokensOnLine[1].text
            + std::string(". (")
            + std::to_string(tokensOnLine[1].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[1].charIndex)
            + std::string(").")
        );
    }

    if (setToken->lineNumber > line)
    {
        while (setToken->lineNumber > line - 10)
        {
            --setToken;
        } // Goes one below. (so we find the first token on that line.)

        // Goes to the first token on that line.
        ++setToken;
    }
    else if (setToken->lineNumber < line)
    {
        while (setToken->lineNumber < line)
        {
            ++setToken;
        }
    }

    isGoto = true;
}

void Goto()
{
    size_t line {0};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (curToken.type == INT)
                {
                    line = stoi(curToken.text);
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("GOTO needs an integer: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
        }
    }

    bool foundLine {false};

    for (const Tok &tok: tokens)
    {
        if (line == tok.lineNumber)
        {
            foundLine = true;
            break;
        }
    }

    if (!foundLine)
    {
        throw std::runtime_error
        (
            std::string("GOTO out of range: ")
            + tokensOnLine[1].text
            + std::string(". (")
            + std::to_string(tokensOnLine[1].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[1].charIndex)
            + std::string(").")
        );
    }

    if (setToken->lineNumber > line)
    {
        while (setToken->lineNumber > line - 10)
        {
            --setToken;
        } // Goes one below. (so we find the first token on that line.)

        // Goes to the first token on that line.
        ++setToken;
    }
    else if (setToken->lineNumber < line)
    {
        while (setToken->lineNumber < line)
        {
            ++setToken;
        }
    }

    isGoto = true;
}

void End()
{
    int exitCode {0};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (curToken.type == INT
                    || curToken.type == DOUBLE)
                {
                    exitCode = stoi(curToken.text);
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("GOTO needs an integer: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
        }
    }

    exit(exitCode);
}

void Portion()
{
    const char *ref {}, *path {};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &token {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (token.type == IDENTIFIER)
                {
                    ref = token.text.c_str();

                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );

            case 2:
                if (token.type == OPERATOR
                    && token.text == ">")
                {
                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );

            case 3:
                if (token.type == STRING)
                {
                    path = token.text.c_str();

                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );
        }
    }

    std::ifstream file {path};
    const std::string contents
    {
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    };

    Lexer lexer {};

    const std::vector<Tok> port {lexer.Parse(contents.c_str(), isDebug)};

    portions[ref] = port;
}

void Release()
{
    if (tokensOnLine.size() < 2 || tokensOnLine.size() > 2)
    {
        throw std::runtime_error
        (
            std::string("Incorrect argument count: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(tokensOnLine[0].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[0].charIndex)
            + std::string(").")
        );
    }

    const char *ref {};

    if (tokensOnLine[1].type == IDENTIFIER)
    {
        ref = tokensOnLine[1].text.c_str();
    }

    if (IsPort(ref))
    {
        Parser parser {isDebug};
        parser.Parse(portions[ref]);
    }
    else
    {
        throw std::runtime_error
        (
            std::string("Portion not found: ")
            + ref
            + std::string(". (")
            + std::to_string(tokensOnLine[1].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[1].charIndex)
            + std::string(").")
        );
    }
}

void Append()
{
    const char *arrName {};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &token {tokensOnLine[i]};

        switch (i)
        {
            case 0: // Do nothing
                break;

            case 1:
                if (IsArr(token.text))
                {
                    arrName = token.text.c_str();

                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Couldn't find array: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );

            case 2:
            {
                Tok tok {};
                tok.lineNumber = token.lineNumber;
                tok.charIndex = token.charIndex;

                if (token.type == IDENTIFIER)
                {
                    if (token.text == "TE_MATH")
                    {
                        const char *_tokens {token.text.c_str()};
                        _tokens = GetVarsInStr(_tokens).c_str();
                        const long double val {Math(_tokens)};

                        if (IsWhole(val) && arrays[arrName].dataType == INT)
                        {
                            tok.text = std::to_string((int)val);
                        }
                        else if (!IsWhole(val) && arrays[arrName].dataType == DOUBLE)
                        {
                            tok.text = std::to_string((double)val);
                        }
                        else
                        {
                            throw std::runtime_error
                            (
                                std::string("Array specifications didn't match: ")
                                + token.text
                                + std::string(". (")
                                + std::to_string(token.lineNumber)
                                + std::string(", ")
                                + std::to_string(token.charIndex)
                                + std::string(").")
                            );
                        }

                        arrays[arrName].children.emplace_back(tok);

                        ++i;
                    }
                    else if (IsVar(token.text))
                    {
                        if (variables[token.text].dataType == arrays[arrName].dataType)
                        {
                            tok.type = arrays[arrName].dataType;
                            tok.text = variables[token.text].text;

                            arrays[arrName].children.emplace_back(tok);
                        }
                    }
                    else
                    {
                        throw std::runtime_error
                        (
                            std::string("Unexpected identifier: ")
                            + token.text
                            + std::string(". (")
                            + std::to_string(token.lineNumber)
                            + std::string(", ")
                            + std::to_string(token.charIndex)
                            + std::string(").")
                        );
                    }
                }
                else
                {
                    if (arrays[arrName].dataType == token.type)
                    {
                        tok.type = token.type;
                        tok.text = token.text;

                        arrays[arrName].children.emplace_back(tok);
                    }
                    else
                    {
                        throw std::runtime_error
                        (
                            std::string("Array type miss-match: ")
                            + token.text
                            + std::string(". (")
                            + std::to_string(token.lineNumber)
                            + std::string(", ")
                            + std::to_string(token.charIndex)
                            + std::string(").")
                        );
                    }
                }
            }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );
        }
    }
}

void StrComb()
{
    const char *varName {}, *tmpText {};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        if (i == 0) { continue; }
        else if (i == 1)
        {
            if (IsVar(curToken.text))
            {
                if (variables[curToken.text].dataType != STRING)
                {
                    throw std::runtime_error
                    (
                        std::string("Origin variable must be a string for STR_COMB: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }

                varName = curToken.text.c_str();

                tmpText = variables[curToken.text].text.c_str();
            }
            else
            {
                throw std::runtime_error
                (
                    std::string("Couldn't find variable: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
            }
        }
        else if (i == 2)
        {
            if (curToken.text != "+")
            {
                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + curToken.text
                    + std::string(". Expected '+' (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
            }
        }
        else if (i == 3)
        {
            std::string result {tmpText};

            if (curToken.type == IDENTIFIER && IsVar(curToken.text))
            {
                if (variables[curToken.text].dataType != STRING)
                {
                    throw std::runtime_error
                    (
                        std::string("Combining variable must be string for STR_COMB: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }

                result += variables[curToken.text].text;

                variables[varName].text = result;
            }
            else if (curToken.type == STRING)
            {
                result += curToken.text;

                variables[varName].text = result;
            }
            else if (curToken.type == INT)
            {
                const int asciiValue {std::stoi(curToken.text)};
                const char c {(char)asciiValue};

                result += c;

                variables[varName].text = result;
            }
            else
            {
                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + curToken.text
                    + std::string(". Expected a string (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
            }
        }
        else
        {
            throw std::runtime_error
            (
                std::string("Incorrect argument count for STR_COMB: ")
                + curToken.text
                + std::string(". Expected argc is 3 (")
                + std::to_string(curToken.lineNumber)
                + std::string(", ")
                + std::to_string(curToken.charIndex)
                + std::string(").")
            );
        }
    }
}

void Free()
{
    if (tokensOnLine.size() != 2)
    {
        throw std::runtime_error
        (
            std::string("Incorrect argument count: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(tokensOnLine[0].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[0].charIndex)
            + std::string(").")
        );
    }

    const Tok &varName {tokensOnLine[1]};

    if (!IsVar(varName.text))
    {
        throw std::runtime_error
        (
            std::string("Could not find variable: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(varName.lineNumber)
            + std::string(", ")
            + std::to_string(varName.charIndex)
            + std::string(").")
        );
    }

    variables.erase(varName.text);
}

void FreeArr()
{
    if (tokensOnLine.size() != 2)
    {
        throw std::runtime_error
        (
            std::string("Incorrect argument count: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(tokensOnLine[0].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[0].charIndex)
            + std::string(").")
        );
    }

    const Tok &arrName {tokensOnLine[1]};

    if (!IsArr(arrName.text))
    {
        throw std::runtime_error
        (
            std::string("Could not find array: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(arrName.lineNumber)
            + std::string(", ")
            + std::to_string(arrName.charIndex)
            + std::string(").")
        );
    }

    arrays.erase(arrName.text);
}

void FreePort()
{
    if (tokensOnLine.size() != 2)
    {
        throw std::runtime_error
        (
            std::string("Incorrect argument count: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(tokensOnLine[0].lineNumber)
            + std::string(", ")
            + std::to_string(tokensOnLine[0].charIndex)
            + std::string(").")
        );
    }

    const Tok &portName {tokensOnLine[1]};

    if (!IsPort(portName.text))
    {
        throw std::runtime_error
        (
            std::string("Could not find portion: ")
            + std::to_string(tokensOnLine.size())
            + std::string(". (")
            + std::to_string(portName.lineNumber)
            + std::string(", ")
            + std::to_string(portName.charIndex)
            + std::string(").")
        );
    }

    portions.erase(portName.text);
}

bool CheckIf(const std::string &lhs, enum BoolOperator op, const std::string &rhs)
{
    bool ret {false};

    int x {stoi(lhs)}, y {stoi(rhs)};

    switch (op)
    {
        case EQUALS:
            ret = (lhs == rhs);
            break;

        case NOT:
            ret = (lhs != rhs);
            break;

        case LESS:
            ret = (x < y);
            break;

        case GREATER:
            ret = (x > y);
            break;
    }

    return ret;
}

bool InArray(const std::string &value, const std::vector<std::string> &array)
{
    bool ret {false};

    for (const std::string &s: array)
    {
        if (value == s)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

bool IsVar(const std::string &value)
{
    auto it {variables.find(value)};
    if (it != variables.end())
    {
        return true;
    }

    return false;
}

bool IsArr(const std::string &value)
{
    auto it {arrays.find(value)};
    if (it != arrays.end())
    {
        return true;
    }

    return false;
}

bool IsPort(const std::string &value)
{
    auto it {portions.find(value)};
    if (it != portions.end())
    {
        return true;
    }

    return false;
}

bool IsWhole(long double d)
{
    /*                 / - From 'long double' to 'double'  - \       */
    if ((d - floor((double)d) < 0.000000001) || (d - floor((double)d) > 0.9999999999))
    {
        return true;
    }

    return false;
}

void ReAssignVar()
{
    const char *var;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                if (IsVar(curToken.text))
                {
                    var = curToken.text.c_str();

                    if (variables[var].isConst)
                    {
                        throw std::runtime_error
                        (
                            std::string("Cannot re-assign const variable: ")
                            + curToken.text
                            + std::string(". (")
                            + std::to_string(curToken.lineNumber)
                            + std::string(", ")
                            + std::to_string(curToken.charIndex)
                            + std::string(").")
                        );
                    }
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Couldn't find variable: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 1:
                if (curToken.type != OPERATOR
                    || curToken.text != ":")
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected token: ")
                        + curToken.text
                        + std::string(". (")
                        + std::to_string(curToken.lineNumber)
                        + std::string(", ")
                        + std::to_string(curToken.charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 2:
                if (curToken.type == OPERATOR
                    && curToken.text == "*")
                {
                    /* Changing data-type */
                    variables[var].dataType = tokensOnLine[i + 1].type;
                    variables[var].text = tokensOnLine[i + 1].text;

                    continue;
                }
                else if (curToken.type == variables[var].dataType)
                {
                    variables[var].text = curToken.text;

                    continue;
                }
                if (curToken.type == IDENTIFIER)
                {
                    if (curToken.text == "TE_MATH")
                    {
                        const char *_tokens {tokensOnLine[i + 1].text.c_str()};
                        _tokens = GetVarsInStr(_tokens).c_str();
                        long double val {Math(_tokens)};

                        if (IsWhole(val) && variables[var].dataType == INT)
                        {
                            variables[var].text = std::to_string((int)val);
                        }
                        else if (!IsWhole(val) && variables[var].dataType == DOUBLE)
                        {
                            variables[var].text = std::to_string((double)val);
                        }
                        else
                        {
                            throw std::runtime_error
                            (
                                std::string("Variable specifications didn't match: ")
                                + curToken.text
                                + std::string(". (")
                                + std::to_string(curToken.lineNumber)
                                + std::string(", ")
                                + std::to_string(curToken.charIndex)
                                + std::string(").")
                            );
                        }

                        ++i;

                        continue;
                    }
                }

                throw std::runtime_error
                (
                    std::string("Type miss-match: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );

            case 3:
                if (curToken.type == variables[var].dataType)
                {
                    const char *token {tokensOnLine[3].text.c_str()};
                    variables[var].text = token;
                }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + curToken.text
                    + std::string(". (")
                    + std::to_string(curToken.lineNumber)
                    + std::string(", ")
                    + std::to_string(curToken.charIndex)
                    + std::string(").")
                );
        }
    }
}

void ReAssignArr()
{
    const char *arr {};
    TokenType arrType {WHITESPACE};
    int index {};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &token {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                if (IsArr(token.text))
                {
                    arr = token.text.c_str();

                    arrType = arrays[token.text].dataType;

                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Could not find array: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );

            case 1:
                if (token.type == INT)
                {
                    index = std::stoi(token.text);
                }
                else if (token.type == IDENTIFIER)
                {
                    if (IsVar(token.text))
                    {
                        if (arrType == variables[token.text].dataType)
                        {
                            index = std::stoi(variables[token.text].text);

                            continue;
                        }

                        throw std::runtime_error
                        (
                            std::string("Unexpected token: ")
                            + token.text
                            + std::string(". expected integer to find index (")
                            + std::to_string(token.lineNumber)
                            + std::string(", ")
                            + std::to_string(token.charIndex)
                            + std::string(").")
                        );
                    }
                }
                break;

            case 2:
                if (token.type != OPERATOR
                    || token.text != ":")
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected token: ")
                        + token.text
                        + std::string(". (")
                        + std::to_string(token.lineNumber)
                        + std::string(", ")
                        + std::to_string(token.charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 3:
                if (token.type == IDENTIFIER)
                {
                    if (token.text == "TE_MATH")
                    {
                        const char *_tokens {tokensOnLine[i + 1].text.c_str()};
                        _tokens = GetVarsInStr(_tokens).c_str();
                        long double val {Math(_tokens)};

                        if (IsWhole(val) && arrays[arr].dataType == INT)
                        {
                            arrays[arr].children[index].text = std::to_string((int)val);
                        }
                        else if (!IsWhole(val) && arrays[arr].dataType == DOUBLE)
                        {
                            arrays[arr].children[index].text = std::to_string((double)val);
                        }
                        else
                        {
                            throw std::runtime_error
                            (
                                std::string("Array specifications didn't match: ")
                                + token.text
                                + std::string(". (")
                                + std::to_string(token.lineNumber)
                                + std::string(", ")
                                + std::to_string(token.charIndex)
                                + std::string(").")
                            );
                        }

                        ++i;

                        continue;
                    }

                    if (IsVar(token.text))
                    {
                        if (arrType == variables[token.text].dataType)
                        {
                            arrays[arr].children[index].text = variables[token.text].text;
                        }
                        else
                        {
                            throw std::runtime_error
                            (
                                std::string("Unexpected token: ")
                                + token.text
                                + std::string(". expected an integer (")
                                + std::to_string(token.lineNumber)
                                + std::string(", ")
                                + std::to_string(token.charIndex)
                                + std::string(").")
                            );
                        }
                    }
                }
                else
                {
                    if (arrType == token.type)
                    {
                        arrays[arr].children[index].text = token.text;
                    }
                    else
                    {
                        throw std::runtime_error
                        (
                            std::string("Type miss-match: ")
                            + token.text
                            + std::string(". (")
                            + std::to_string(token.lineNumber)
                            + std::string(", ")
                            + std::to_string(token .charIndex)
                            + std::string(").")
                        );
                    }
                }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments in array re-assign: ")
                    + token.text
                    + std::string(". (")
                    + std::to_string(token.lineNumber)
                    + std::string(", ")
                    + std::to_string(token.charIndex)
                    + std::string(").")
                );
        }
    }
}

long double Math(const char *expr)
{
    te_parser tep;

    const long double ret {tep.evaluate(expr)};

    if (tep.success())
    {
        return ret;
    }

    throw std::runtime_error
    (
        std::string
        (
            "TE_MATH failed in function, cannot reach token values..."
        )
    );
}

std::string GetVarsInStr(const char *str)
{
    std::string ret {}, delim {};

    int stage {0};

    size_t startIndex {}, len {strlen(str)};

    for (size_t i {0}; i < len; ++i)
    {
        const char c {str[i]};

        switch (c)
        {
            case '[':
            case '{':
                stage = 1;
                startIndex = i;
                break;

            case '}':
                stage = 0;

                if (IsVar(delim))
                {
                    ret.insert(startIndex, variables[delim].text);
                }

                delim.erase();
                break;

            case ']':
            {
                stage = 0;

                std::string indexArr {}, indexStr {};

                for (const char ch: delim)
                {
                    if (ch == ';')
                    {
                        stage = 1;

                        continue;
                    }

                    if (stage == 1)
                        indexStr += ch;
                    else
                        indexArr += ch;
                }

                if (IsArr(indexArr))
                {
                    unsigned int index {};

                    std::stringstream ss {indexStr};
                    if (ss >> index) {}
                    else if (IsVar(indexStr)
                             && variables[indexStr].dataType == INT)
                    {
                        index = std::stoi(variables[indexStr].text);
                    }
                    else
                    {
                        throw std::runtime_error
                        (
                            std::string
                            (
                            "index needs to be an integer"
                            )
                        );
                    }

                    ret.insert(startIndex, arrays[indexArr].children[index].text);
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Cannot find array: \"")
                        + indexArr
                        + std::string("\".")
                    );
                }
            }

                stage = 0;

                break;

            default:
                if (stage == 1)
                    delim += c;
                else
                    ret += c;
                break;
        }
    }

    return ret;
}