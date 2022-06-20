#include <iostream>
#include <string>
#include <vector>
#include <fstream>

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

    Lexer lexer {};

    std::vector<Token> tokens {lexer.Parse(contents)};

    if (debug)
        std::cout << "\n" << tokens.size() << " tokens found:\n{\n\n";

    if (debug)
    {
        for (size_t i {0}; i < tokens.size(); ++i)
        {
            if (tokens[i].type == EOL)
            {
                std::cout << "\n\tNEWLINE\n\n";

                continue;
            }

            std::cout
                    << "\t"
                    << TokenTypeStrings[tokens[i].type]
                    << ": \""
                    << tokens[i].text
                    << "\", ("
                    << tokens[i].lineNumber
                    << ", "
                    << tokens[i].charIndex
                    << ");\n";
        }

        std::cout << "\n}\n";
    }

    Parser parser {debug};

    if (debug)
        std::cout << "Output: \n{\n\n";

    try
    {
        parser.Parse(tokens);
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Parser error occurred: \n\t" << e.what() << "\n";
    }

    if (debug)
        std::cout << "\n\n}\n";

    return 0;
}