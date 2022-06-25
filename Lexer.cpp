#include "Lexer.h"

std::vector<Tok> Lexer::Parse(const std::string &file, bool _isDebug)
{
    isDebug = _isDebug;

    const size_t len {file.length()};
    char program[len + 1];

    strcpy(program, file.c_str());

    for (size_t i {0}; i < len; ++i)
    {
        const char c {program[i]};

        ++curToken.charIndex;

        if (curToken.type == MAYBE_COMMENT && c != '/')
        {
            curToken.type = OPERATOR;

            NewToken();

            continue;
        }
        else if (curToken.type == SES)
        {
            switch (c)
            {
                case 'n':
                    curToken.text += '\n';
                    break;

                case 'r':
                    curToken.text += '\r';
                    break;

                case 't':
                    curToken.text += '\t';
                    break;

                default:
                    std::cerr << "Lexer error:\n\tUnknown escape sequence '\\" << c << "', ("
                        << curToken.lineNumber << ", " << curToken.charIndex << ").\n";

                    exit(1);
                    break;
            }

            curToken.type = STRING;

            continue;
        }

        switch (c)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (curToken.type == WHITESPACE)
                {
                    curToken.type = INT;
                    curToken.text += c;
                }
                else if (curToken.type == MAYBE_DOUBLE)
                {
                    curToken.type = DOUBLE;
                    curToken.text += c;
                }
                else
                {
                    curToken.text += c;
                }
                break;

            case '.':
                if (curToken.type == WHITESPACE)
                {
                    curToken.type = MAYBE_DOUBLE;
                    curToken.text += c;
                }
                else if (curToken.type == INT)
                {
                    curToken.type = DOUBLE;
                    curToken.text += c;
                }
                else if (curToken.type == STRING)
                {
                    curToken.text += c;
                }
                else
                {
                    NewToken();

                    curToken.type = OPERATOR;
                    curToken.text += c;

                    NewToken();
                }
                break;

            case '{':
            case '}':
            case '(':
            case ')':
            case '=':
            case '!':
            case '?':
            case '+':
            case '-':
            case '*':
            case '<':
            case '>':
            case ':':
            case ';':
            case ',':
                if (curToken.type == STRING)
                {
                    curToken.text += c;
                }
                else
                {
                    NewToken();

                    curToken.type = OPERATOR;
                    curToken.text += c;

                    NewToken();
                }
                break;

            case ' ' :
            case '\t':
                if (curToken.type == STRING
                    || curToken.type == COMMENT)
                {
                    curToken.text += c;
                }
                else
                {
                    NewToken();
                }
                break;

            case '\n':
            case '\r':
                if (curToken.type == STRING)
                {
                    curToken.text += c;
                }
                else
                {
                    NewToken();

                    curToken.type = EOL;
                    curToken.text = '\n';

                    NewToken();

                    curToken.lineNumber += 10;
                    curToken.charIndex = 1;
                }
                break;

            case '"':
                if (curToken.type == STRING)
                {
                    NewToken();
                }
                else
                {
                    NewToken();
                    curToken.type = STRING;
                }
                break;

            case '/':
                if (curToken.type == STRING)
                {
                    curToken.text += c;
                }
                else if (curToken.type == MAYBE_COMMENT)
                {
                    curToken.type = COMMENT;
                    curToken.text.erase();
                }
                else
                {
                    NewToken();

                    curToken.type = MAYBE_COMMENT;
                    curToken.text += c;
                }
                break;

            case '\\':
                if (curToken.type == STRING)
                {
                    curToken.type = SES;
                }
                else
                {
                    NewToken();

                    curToken.type = OPERATOR;
                    curToken.text += c;

                    NewToken();
                }
                break;

            default:
                if (curToken.type == WHITESPACE
                    || curToken.type == INT
                    || curToken.type == DOUBLE)
                {
                    NewToken();

                    curToken.type = IDENTIFIER;
                    curToken.text += c;
                }
                else
                {
                    curToken.text += c;
                }
        }
    }

    NewToken();

    return tokens;
}

void Lexer::NewToken()
{
    if (curToken.type == COMMENT && isDebug)
    {
        std::cout << "/*\n" << curToken.text << "\n*/\n";
    }
    else if (curToken.type != WHITESPACE)
    {
        tokens.push_back(curToken);
    }

    if (curToken.type == MAYBE_DOUBLE)
    {
        if (curToken.text == ".")
        {
            curToken.type = OPERATOR;
        }
        else
        {
            curToken.type = DOUBLE;
        }
    }

    curToken.type = WHITESPACE;
    curToken.text.erase();
}