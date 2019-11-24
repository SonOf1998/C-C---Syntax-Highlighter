#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "CSrcHighlighter.h"
#include "CppSrcHighlighter.h"


#define TEST_OUTPUT "highlighted_src.html"

/*
 * Gyengeségek:
 *
 * A @ X ~ ` katakterek sehol nem fordulhatnak elő a forráskódban. Jobb esetben csak nem jelennek meg, rosszabb esetben
 * megtörik a html formázást.
 * A string literálokban és kommentekben nem fordulhatnak elő html tagszerű elemek (<...>), include-ok kivételt képeznek.
 * String literálok utáni számkonstansok már nem (mindig) lesznek kikékítve.
 * Hexák rossz színezéssel jelennek meg. *
 * egyéb
 *
 *
 *
 * test.c
 * test.cpp
 *
 *
 */


int main()
{
    std::cout << "Full path of C/C++ source code file: " << std::endl;
    std::string path;
    std::getline(std::cin, path);       // szóközös mappákra

    std::vector<std::string> file;
    std::string line;
    std::ifstream is(std::cref(path));

    while (std::getline(is, line))
    {
        file.push_back(std::move(line));
    }
    is.close();

    const std::unique_ptr<CSrcHighlighter> highlighter = std::move (
            [&]() -> std::unique_ptr<CSrcHighlighter>
            {
                // .h/.hpp fájlokat nem fogad
                std::regex cFileRegex(R"(.*\.c)");
                std::regex cppFileRegex(R"(.*\.((cc)|(cpp)|(cxx)))");


                if (std::regex_match(path, cFileRegex))
                {
                    return std::make_unique<CSrcHighlighter>(file);
                }
                else if (std::regex_match(path, cppFileRegex))
                {
                    return std::make_unique<CppSrcHighlighter>(file);
                }

                throw std::runtime_error("Unknown file format!");
            }()
    );

    highlighter->exportHighlightedSrcCode(TEST_OUTPUT);
}

