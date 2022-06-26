#include <iostream>
#include <string>
#include <vector>

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"

int main(int argc, char **argv)
{
    if (argc > 3 || argc < 2)
    {
        std::cerr << "Path to file not given...\n";

        return 1;
    }

    bool debug {false};

    if (argc == 3)
    {
        std::string d {argv[2]};

        if (d == "-d")
            debug = true;
        else
        {
            std::cerr << "Unknown 3rd argument (" << d << ")...\n";

            return 1;
        }
    }

    char *path {argv[1]};

    std::ifstream file {path};
    std::string contents
    {
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
    };

    if (debug)
        std::cout << "FILE:\n{\n\n" << contents << "\n\n}\n";

    Lexer *lexer {new Lexer {}};

    std::vector<Tok> lexedTokens {lexer->Parse(contents, debug)};

    delete lexer;

    if (debug)
        std::cout << "\n" << lexedTokens.size() << " lexedTokens found:\n{\n\n";

    if (debug)
    {
        for (size_t i {0}; i < lexedTokens.size(); ++i)
        {
            if (lexedTokens[i].type == EOL)
            {
                std::cout << "\n\tNEWLINE\n\n";

                continue;
            }

            std::cout
                    << "\t"
                    << TokenTypeStrings[lexedTokens[i].type]
                    << ": \""
                    << lexedTokens[i].text
                    << "\", ("
                    << lexedTokens[i].lineNumber
                    << ", "
                    << lexedTokens[i].charIndex
                    << ");\n";
        }

        std::cout << "\n}\n";
    }

    Parser *parser {new Parser{debug}};

    if (debug)
        std::cout << "Output: \n{\n\n";

    try
    {
        parser->Parse(lexedTokens);
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Parser error occurred: \n\t" << e.what() << "\n";
    }

    delete parser;

    if (debug)
        std::cout << "\n\n}\n";

    return 0;
}