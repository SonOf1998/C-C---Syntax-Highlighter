#include "CppSrcHighlighter.h"

const char* CppSrcHighlighter::getKeywordsTxtPath() const noexcept
{
    return CPP_KEYWORDS_TXT_PATH;
}


void CppSrcHighlighter::highlightNamespaces()
{
    for (auto& line : file)
    {
        std::regex regexForNamespaces("[[:alpha:]]*_?[[:alpha:]]+::");
        line = std::regex_replace(line, regexForNamespaces, "<span style=\"color:#007FA2;\">$0</span>");
    }
}

void CppSrcHighlighter::highlightTemplateParams()
{
    for (int i = 0; i < static_cast<int>(file.size()); ++i)
    {
        auto& originalLine = unmodifiedFile[i];
        auto& line = file[i];


        bool templateWasInclude = false;    // <std::string> != <iostream>
        for (char c : originalLine) 
        {
            if (c == '#')
            {
                templateWasInclude = true;
                break;
            }
            else if (c != ' ')
            {
                break;
            }
        }
        
        if (!templateWasInclude)
        {
            std::regex regexForTemplateArgs("<[[:alpha:]]+>");
            line = std::regex_replace(line, regexForTemplateArgs, "~$0`");
            line = std::regex_replace(line, std::regex("~<"), "<span style=\"color:#007FA2;\">&lt");
            line = std::regex_replace(line, std::regex(">`"), "&gt</span>");
        }
    }
}

void CppSrcHighlighter::highlightProgrammingLangSpecificStuff()
{
    highlightNamespaces();
    highlightTemplateParams();
}





