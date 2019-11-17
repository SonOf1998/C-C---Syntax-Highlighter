#ifndef CPPHF2_CSRCHIGHLIGHTER_H
#define CPPHF2_CSRCHIGHLIGHTER_H

#include <iostream>
#include <regex>
#include <sstream>

#include "SrcHighlighter.h"

#define C_KEYWORDS_TXT_PATH "c_keywords.txt"

class CSrcHighlighter final : public SrcHighlighter
{
    [[nodiscard]] const char* getKeywordsTxtPath() const noexcept override;

public:

    using SrcHighlighter::SrcHighlighter;
};


#endif
