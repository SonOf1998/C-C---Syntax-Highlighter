#ifndef CPPHF2_CPPSRCHIGHLIGHTER_H
#define CPPHF2_CPPSRCHIGHLIGHTER_H

#include <regex>

#include "SrcHighlighter.h"

#define CPP_KEYWORDS_TXT_PATH "cpp_keywords.txt";

class CppSrcHighlighter final : public SrcHighlighter
{
    [[nodiscard]] const char* getKeywordsTxtPath() const noexcept override;


    void highlightTemplateParams();
    void highlightNamespaces();
    void highlightProgrammingLangSpecificStuff() override;

public:

    using SrcHighlighter::SrcHighlighter;


};


#endif
