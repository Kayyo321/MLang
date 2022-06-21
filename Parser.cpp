#include "Parser.h"

bool isGoto {false};
bool isIf {false};

std::vector<std::string> scopeVars;

Parser::Parser(bool _isDebug)
{
    isDebug = _isDebug;

    strFunc["PRINT"] = &Print;
    strFunc["LET"] = &Let;
    strFunc["IF"] = &If;
    strFunc["ENDIF"] = &EndIf;
    strFunc["GOTO"] = &Goto;
    strFunc["END"] = &End;
}

std::vector<Tok>::iterator setToken;

void Parser::Parse(std::vector<Tok> &_tokens)
{
    endToken = _tokens.end();
    curToken = _tokens.begin();
    tokens = _tokens;

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
        std::cout << "DEBUG\n\n";

        for (auto const &pair: variables)
        {
            std::cout << pair.first << ", has the value of: " << pair.second.text << "\n"
                << pair.first << ", has the data-type of: " << TokenTypeStrings[pair.second.dataType] << "\n";
        }
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
                }
                else if (tokensOnLine[i + 1].type == OPERATOR
                         && tokensOnLine[i + 1].text != ":")
                {
                    const std::string &varName {tokensOnLine[0].text};
                    signed long int newVal {Math()};

                    variables[varName].text = std::to_string(newVal);
                }
                break;
            }
        }
    }
}

std::string Print()
{
    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        if (i == 0) continue;

        const Tok curToken {tokensOnLine[i]};

        if (curToken.type == IDENTIFIER)
        {
            if (InArray(curToken.text, Funcs))
            {
                /* Function call */

                std::cout << strFunc[curToken.text]();

                continue;
            }

            if (IsVar(curToken.text))
            {
                std::cout << variables[curToken.text].text;

                continue;
            }

            throw std::runtime_error
            (
                std::string("Unknown identifier: ")
                + tokensOnLine[i].text
                + std::string(". (")
                + std::to_string(tokensOnLine[i].lineNumber)
                + std::string(", ")
                + std::to_string(tokensOnLine[i].charIndex)
                + std::string(").")
            );
        }
        else
        {
            std::cout << curToken.text;
        }
    }

    return {};
}

std::string Let()
{
    Variable x;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (tokensOnLine[1].type == IDENTIFIER)
                {
                    x.name = tokensOnLine[1].text;
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Unknown variable name: ")
                        + tokensOnLine[1].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[1].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[1].charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 2:
                if (tokensOnLine[2].type == OPERATOR
                    && tokensOnLine[2].text == ":")
                {
                    continue;
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Unknown operator: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }

            case 3:
                x.dataType = tokensOnLine[3].type;
                x.text = tokensOnLine[3].text;
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + tokensOnLine[i].text
                    + std::string(". (")
                    + std::to_string(tokensOnLine[i].lineNumber)
                    + std::string(", ")
                    + std::to_string(tokensOnLine[i].charIndex)
                    + std::string(").")
                );
        }
    }

    variables[x.name] = x;

    if (isIf)
        scopeVars.push_back(x.name);

    return {};
}

std::string If()
{
    std::string lhs;
    BoolOperator op;
    std::string rhs;

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
                    if (InArray(curToken.text, Funcs))
                    {
                        /* Function call */

                        lhs = (strFunc[curToken.text]());

                        break;
                    }

                    if (IsVar(curToken.text))
                    {
                        lhs = (variables[curToken.text].text);

                        break;
                    }

                    throw std::runtime_error
                    (
                        std::string("Unexpected Identifier: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                else
                {
                    lhs = curToken.text;
                }
                break;

            case 2:
                if (curToken.type != OPERATOR)
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected token: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
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
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 3:
                if (curToken.type == IDENTIFIER)
                {
                    if (InArray(curToken.text, Funcs))
                    {
                        /* Function call */

                        rhs = (strFunc[curToken.text]());

                        break;
                    }

                    if (IsVar(curToken.text))
                    {
                        rhs = (variables[curToken.text].text);

                        break;
                    }

                    throw std::runtime_error
                    (
                        std::string("Unexpected Identifier: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                else
                {
                    rhs = curToken.text;
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
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                break;
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
    return {};
}

std::string EndIf()
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

    return {};
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

std::string Goto()
{
    size_t line {0};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (tokensOnLine[1].type == INT)
                {
                    line = stoi(tokensOnLine[1].text);
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("GOTO needs an integer: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                break;

            default:
                throw std::runtime_error
                (
                    std::string("Too many arguments: ")
                    + tokensOnLine[i].text
                    + std::string(". (")
                    + std::to_string(tokensOnLine[i].lineNumber)
                    + std::string(", ")
                    + std::to_string(tokensOnLine[i].charIndex)
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

    return {};
}

std::string End()
{
    double exitCode {};

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        switch (i)
        {
            case 0:
                continue;

            case 1:
                if (tokensOnLine[1].type == INT
                    || tokensOnLine[1].type == DOUBLE)
                {
                    exitCode = stoi(tokensOnLine[1].text);
                }
                else
                {
                    throw std::runtime_error
                            (
                                    std::string("GOTO needs an integer: ")
                                    + tokensOnLine[i].text
                                    + std::string(". (")
                                    + std::to_string(tokensOnLine[i].lineNumber)
                                    + std::string(", ")
                                    + std::to_string(tokensOnLine[i].charIndex)
                                    + std::string(").")
                            );
                }
                break;

            default:
                throw std::runtime_error
                        (
                                std::string("Too many arguments: ")
                                + tokensOnLine[i].text
                                + std::string(". (")
                                + std::to_string(tokensOnLine[i].lineNumber)
                                + std::string(", ")
                                + std::to_string(tokensOnLine[i].charIndex)
                                + std::string(").")
                        );
        }
    }

    exit(exitCode);
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

void ReAssignVar()
{
    std::string var;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        switch (i)
        {
            case 0:
                var = tokensOnLine[0].text;
                break;

            case 1:
                if (tokensOnLine[1].type != OPERATOR
                    || tokensOnLine[1].text != ":")
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected token: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 2:
                if (tokensOnLine[2].type == OPERATOR
                    && tokensOnLine[2].text == "*")
                {
                    /* Changing data-type */
                    variables[var].dataType = tokensOnLine[3].type;
                    variables[var].text = tokensOnLine[3].text;
                }
                else if (tokensOnLine[2].type == variables[var].dataType)
                {
                    variables[var].text = tokensOnLine[2].text;
                }
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Type miss-match: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                break;
        }
    }
}

void ParseMath(const std::string &lhs, Operator op, const std::string &rhs, signed long int &ref)
{
    int x {stoi(lhs)}, y {stoi(rhs)};

    switch (op)
    {
        case ADD:
            ref = x + y;
            break;

        case SUB:
            ref = x - y;
            break;

        case MUL:
            ref = x * y;
            break;

        case DIV:
            ref = x / y;
            break;

        case MOD:
            ref = x % y;
            break;
    }
}

signed long int Math()
{
    signed long int ret {};

    std::string lhs, rhs;
    Operator op;

    for (size_t i {0}; i < tokensOnLine.size(); ++i)
    {
        const Tok &curToken {tokensOnLine[i]};

        switch (i)
        {
            case 0:
                if (curToken.type == IDENTIFIER)
                {
                    if (InArray(curToken.text, Funcs))
                    {
                        /* Function call */

                        lhs = (strFunc[curToken.text]());

                        break;
                    }

                    if (IsVar(curToken.text))
                    {
                        lhs = (variables[curToken.text].text);

                        break;
                    }

                    throw std::runtime_error
                            (
                                    std::string("Unexpected Identifier: ")
                                    + tokensOnLine[i].text
                                    + std::string(". (")
                                    + std::to_string(tokensOnLine[i].lineNumber)
                                    + std::string(", ")
                                    + std::to_string(tokensOnLine[i].charIndex)
                                    + std::string(").")
                            );
                }
                else
                {
                    lhs = curToken.text;
                }
                break;

            case 1:
                if (curToken.type != OPERATOR)
                {
                    throw std::runtime_error
                            (
                                    std::string("Unexpected token: ")
                                    + tokensOnLine[i].text
                                    + std::string(". (")
                                    + std::to_string(tokensOnLine[i].lineNumber)
                                    + std::string(", ")
                                    + std::to_string(tokensOnLine[i].charIndex)
                                    + std::string(").")
                            );
                }

                if (curToken.text == "+")
                    op = ADD;
                else if (curToken.text == "-")
                    op = SUB;
                else if (curToken.text == "*")
                    op = MUL;
                else if (curToken.text == "/")
                    op = DIV;
                else if (curToken.text == "%")
                    op = MOD;
                else
                {
                    throw std::runtime_error
                    (
                        std::string("Unexpected operator: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 2:
                if (curToken.type == IDENTIFIER)
                {
                    if (InArray(curToken.text, Funcs))
                    {
                        /* Function call */

                        rhs = (strFunc[curToken.text]());

                        break;
                    }

                    if (IsVar(curToken.text))
                    {
                        rhs = (variables[curToken.text].text);

                        break;
                    }

                    throw std::runtime_error
                    (
                        std::string("Unexpected Identifier: ")
                        + tokensOnLine[i].text
                        + std::string(". (")
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }
                else
                {
                    rhs = curToken.text;
                }
                break;
        }
    }

    ParseMath(lhs, op, rhs, ret);

    return ret;
}