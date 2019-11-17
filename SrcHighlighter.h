#ifndef CPPHF2_SRCHIGHLIGHTER_H
#define CPPHF2_SRCHIGHLIGHTER_H

/* <span style="..."></span>
 *
 * keywords & numeric consts & escapes: "color:#0000ff;"
 * comment:                             "color:#666666;"
 * str & preprocessor dir msg:          "color:#339900;"
 * template parameter & namespace:      "color:#007788;"
 * preprocessor dir:                    "color:#C19600;"
 *
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#define PREPROCESSOR_FILE_PATH "preprocessor.txt"

class SrcHighlighter
{
    std::vector<std::string>& file;

    std::vector<std::string> keywords;
    std::vector<std::string> prep_dirs;

    [[nodiscard]] virtual const char* getKeywordsTxtPath() const noexcept = 0;

    void importPreprocessorDirectives();
    virtual void importKeywords();
    void doSyntaxHighlighting();

    void highlightCharsAndStringLiterals();
    void highlightEscapes();
    void highlightKeywords();
    void highlightPreprocessorDirectives();
    void highlightNumericConstants();
    void handleSingleLineComments();
    void handleMultiLineComments();

public:

    explicit SrcHighlighter(std::vector<std::string>&) noexcept;
    virtual ~SrcHighlighter() = default;


    void exportHighlightedSrcCode(const std::string&);
};


#endif
