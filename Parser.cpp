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

            if (IsArr(tok.text))
            {
                ReAssignArr();
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

            if (IsArr(curToken.text))
            {
                if (tokensOnLine[i + 1].type == INT)
                {
                    int index {std::stoi(tokensOnLine[i + 1].text)};

                    std::cout << arrays[curToken.text].children[index].text;

                    ++i;
                }
                else if (IsVar(tokensOnLine[i + 1].text))
                {
                    if (variables[tokensOnLine[i + 1].text].dataType == INT)
                    {
                        int index {std::stoi(variables[tokensOnLine[i + 1].text].text)};

                        std::cout << arrays[curToken.text].children[std::stoi(variables[tokensOnLine[i + 1].text].text)].text;

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
                        + std::to_string(tokensOnLine[i].lineNumber)
                        + std::string(", ")
                        + std::to_string(tokensOnLine[i].charIndex)
                        + std::string(").")
                    );
                }

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

std::string Arr()
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
            if (tokensOnLine[1].type == IDENTIFIER)
            {
                x.name = token.text;
                continue;
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
        }

        if (i == 2)
        {
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
                    + std::string(". Expected ':' (")
                    + std::to_string(tokensOnLine[i].lineNumber)
                    + std::string(", ")
                    + std::to_string(tokensOnLine[i].charIndex)
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
                                    + tokensOnLine[i].text
                                    + std::string(". (")
                                    + std::to_string(tokensOnLine[i].lineNumber)
                                    + std::string(", ")
                                    + std::to_string(tokensOnLine[i].charIndex)
                                    + std::string(").")
                                );
                            }

                            x.children.push_back(token);

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
                        if (x.dataType != token.type)
                        {
                            throw std::runtime_error
                            (
                                std::string("Unexpected data-type: (")
                                + tokensOnLine[i].text
                                + std::string(", ")
                                + TokenTypeStrings[tokensOnLine[i].type]
                                + std::string(") against: (")
                                + TokenTypeStrings[x.dataType]
                                + std::string("). (")
                                + std::to_string(tokensOnLine[i].lineNumber)
                                + std::string(", ")
                                + std::to_string(tokensOnLine[i].charIndex)
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
        scopeArrs.push_back(x.name);

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
    double exitCode {0};

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

std::string Portion()
{
    std::string ref {}, path {};

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
                    ref = token.text;

                    continue;
                }

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

            case 2:
                if (token.type == OPERATOR
                    && token.text == ">")
                {
                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + tokensOnLine[i].text
                    + std::string(". Expected an operator: '>' (")
                    + std::to_string(tokensOnLine[i].lineNumber)
                    + std::string(", ")
                    + std::to_string(tokensOnLine[i].charIndex)
                    + std::string(").")
                );

            case 3:
                if (token.type == STRING)
                {
                    path = token.text;

                    continue;
                }

                throw std::runtime_error
                (
                    std::string("Unexpected token: ")
                    + tokensOnLine[i].text
                    + std::string(". Expected a string (")
                    + std::to_string(tokensOnLine[i].lineNumber)
                    + std::string(", ")
                    + std::to_string(tokensOnLine[i].charIndex)
                    + std::string(").")
                );
        }
    }

    std::ifstream file {path};
    std::string contents
    {
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    };

    Lexer lexer {};

    const std::vector<Tok> port {lexer.Parse(contents, isDebug)};

    portions[ref] = port;

    return {};
}

std::string Release()
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

    std::string ref {};

    if (tokensOnLine[1].type == IDENTIFIER)
    {
        ref = tokensOnLine[1].text;
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

    return {};
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
                    const std::string &token {tokensOnLine[2].text};
                    variables[var].text = token;
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

void ReAssignArr()
{
    std::string arr {};
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
                    arr = token.text;

                    arrType = arrays[token.text].dataType;
                }
                else
                {
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
                }
                break;

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
                        }
                        else
                        {
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
                        + std::to_string(token .charIndex)
                        + std::string(").")
                    );
                }
                break;

            case 3:
                if (token.type == IDENTIFIER)
                {
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