#ifndef CPPHF2_CPPSRCHIGHLIGHTER_H
#define CPPHF2_CPPSRCHIGHLIGHTER_H

#include <regex>

#include "SrcHighlighter.h"

#define CPP_KEYWORDS_TXT_PATH "cpp_keywords.txt";

class CppSrcHighlighter : public SrcHighlighter
{
    [[nodiscard]] const char* getKeywordsTxtPath() const noexcept override;

public:

    using SrcHighlighter::SrcHighlighter;


};


#endif
