#include "SrcHighlighter.h"

SrcHighlighter::SrcHighlighter(std::vector<std::string>& file) noexcept : file{file}, unmodifiedFile{file}
{

}


void SrcHighlighter::importKeywords()
{
    std::ifstream is(getKeywordsTxtPath());
    std::string line;

    while (std::getline(is, line))
    {
        keywords.push_back(std::move(line));
    }
    is.close();
}

void SrcHighlighter::importPreprocessorDirectives()
{
    std::ifstream is(PREPROCESSOR_FILE_PATH);
    std::string line;

    while (std::getline(is, line))
    {
        prep_dirs.push_back(std::move(line));
    }
    is.close();
}

void SrcHighlighter::doSyntaxHighlighting()
{
    highlightCharsAndStringLiterals();
    highlightNumericConstants();
    highlightKeywords();
    highlightProgrammingLangSpecificStuff();
    doubleCheckStringLiterals();
    highlightEscapes();
    highlightPreprocessorDirectives();
    handleSingleLineComments();
    handleMultiLineComments();
}

void SrcHighlighter::exportHighlightedSrcCode(const std::string& str)
{
    importKeywords();
    importPreprocessorDirectives();
    doSyntaxHighlighting();

    std::ofstream of(str);

    of << "<pre>" << '\n';
    for (auto& line : file)
    {
        of << line << '\n';
    }
    of << "</pre>" << "\n";
}

void SrcHighlighter::highlightCharsAndStringLiterals()
{
    for (auto& line : file)
    {
        std::regex detectApostropheIncludes(R"((".*\.h")|(".*\.hpp"))");
        if (!std::regex_search(line, detectApostropheIncludes))
        {
            std::regex regexForStringLiterals(R"("[^"]*"|'[^']*')");
            line = std::regex_replace(line, regexForStringLiterals, "<span style=\"color:#339900;\">$0</span>");
        }
    }
}

void SrcHighlighter::highlightEscapes()
{
    for (auto& line : file)
    {
        line = std::regex_replace(line, std::regex(R"(\\t|\\r|\\n)"), "<span style=\"color:#000000;\"><b>$0</b></span>");
    }
}

void SrcHighlighter::highlightKeywords()
{
    for (const auto& keyword : keywords)
    {
        bool modificationMade = true;

        while (modificationMade)
        {
            std::stringstream ss;
            ss << "[ \t\\)\\(;=]" << keyword << "[ \t\\(\\)\\*;]|^" << keyword << "[ \t\\(\\)\\*;]|[ \t\\)\\(;=]" << keyword << "$|^" << keyword
               << "$";

            std::string keywordRegexStr = ss.str();
            std::regex keywordRegex(keywordRegexStr);

            bool modified = false;
            for (auto &line : file)
            {
                if (line.find(keyword) != std::string::npos)
                {
                    std::string tmp = std::regex_replace(line, keywordRegex, "~$0`");


                    bool lookingForRange = true;
                    for(int i = 0; i < static_cast<int>(tmp.size()); ++i)
                    {
                        if (tmp[i] == '~')
                        {
                            lookingForRange = false;
                        }

                        if (tmp[i] == '`')
                        {
                            lookingForRange = true;
                        }

                        if (!lookingForRange)
                        {
                            if (tmp.substr(i + 1, keyword.size()) == keyword)
                            {
                                for(int j = 0; j < static_cast<int>(keyword.size()); ++j)
                                {
                                    tmp[i + 1 + j] = '%';
                                }

                                lookingForRange = true;
                            }
                        }
                    }

                    tmp.erase(std::remove(tmp.begin(), tmp.end(), '~'), tmp.end());
                    tmp.erase(std::remove(tmp.begin(), tmp.end(), '`'), tmp.end());



                    std::regex keywordSlotRegex(std::string(keyword.size(), '%'));
                    std::stringstream coloredString;
                    coloredString << "<span style=\"color:#0000ff;\">" << keyword << "</span>";

                    tmp = std::regex_replace(tmp, keywordSlotRegex, coloredString.str());


                    if(tmp != line)
                    {
                        modified = true;
                        line = std::move(tmp);
                    }
                }
            }

            modificationMade = modified;
        }
    }
}

void SrcHighlighter::highlightPreprocessorDirectives()
{
    for (const auto& prep_dir : prep_dirs)
    {
        for (auto& line : file)
        {
            char firstNonWhiteSpace = line[line.find_first_not_of(' ')];
            if (firstNonWhiteSpace == '#')
            {
                line = std::regex_replace(line, std::regex(">"), "&gt");
                line = std::regex_replace(line, std::regex("<"), "&lt");

                std::stringstream regexSS;
                regexSS << "#" << prep_dir << " |#" << prep_dir << "$|^" << prep_dir;

                line = std::regex_replace(line, std::regex(regexSS.str()), "<span style=\"color:#C19600;\">$0</span>`");

                std::string preprocessorParamRegexStr("`[^ ]* |`[^ ]*$");
                line = std::regex_replace(line, std::regex(preprocessorParamRegexStr), "<span style=\"color:#339900;\">$0</span>");

                line.erase(std::remove(line.begin(), line.end(), '`'), line.end());
            }
        }
    }
}

void SrcHighlighter::highlightNumericConstants()
{
    for(auto& line : file)
    {
        std::regex regexForStringEncapsulutadNums(R"("[^"]*[[:digit:]]+[^"]*")");
        if (!std::regex_search(line, regexForStringEncapsulutadNums))
        {
            std::regex regexForNumericConst("^[[:digit:]]*\\.?[[:digit:]]+f?|^[[digit]]+|[ \t\\)\\(><=+-/\\\\[*;][[:digit:]]*\\.?[[:digit:]]+f?|[ \t\\)\\(><=+-/\\\\[*;][[digit]]+");
            line = std::regex_replace(line, regexForNumericConst, "~$0`");

            bool modificationStart = false;
            bool firstDigitFound = false;
            for (int i = 0; i < static_cast<int>(line.size()); ++i)
            {
                if (line[i] == '~')
                {
                    modificationStart = true;
                    continue;
                }
                else if(line[i] == '`')
                {
                    modificationStart = false;
                    if (firstDigitFound)
                    {
                        line[i] = 'X';
                    }
                    firstDigitFound = false;
                }

                if (modificationStart)
                {
                    if ((std::isdigit(line[i]) || line[i] == '.') && !firstDigitFound)
                    {
                        line.insert(i,"@");
                        firstDigitFound = true;
                    }
                    else if(firstDigitFound && !std::isdigit(line[i]) && line[i] != '.' && line[i] != 'f')
                    {
                        line[i] = 'X';
                    }
                }
            }

            line = std::regex_replace(line, std::regex("@"), "<span style=\"color:#0000ff;\">");
            line = std::regex_replace(line, std::regex("X"), "</span>");

            line.erase(std::remove(line.begin(), line.end(), '~'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '`'), line.end());
        }
    }
}

void SrcHighlighter::handleSingleLineComments()
{
    for(auto& line : file)
    {
        int commentLocation = line.find("//");

        if (commentLocation != std::string::npos)
        {
            std::string commentRegexStr = line.substr(commentLocation);
            commentRegexStr = std::regex_replace(commentRegexStr, std::regex(R"(\(|\)|\[|\])"), "\\$0");

            std::regex commentRegex(commentRegexStr);
            line = std::regex_replace(line, commentRegex, "<span style=\"color:#666666;\">$0</span>");

            std::string delimiter = "<span style=\"color:#666666;\">";
            std::vector<std::string> commentSeparatedStrings;
            int pos;
            std::string slice;
            while ((pos = line.find(delimiter)) != std::string::npos)
            {
                slice = line.substr(0, pos);
                commentSeparatedStrings.push_back(std::move(slice));
                line.erase(0, pos + delimiter.size());
            }
            commentSeparatedStrings.push_back(std::move(line));

            commentSeparatedStrings[1] = std::regex_replace(commentSeparatedStrings[1], std::regex("<span.{23}>|</span>"), "");
            std::string commentedLine(std::move(commentSeparatedStrings[0]));
            commentedLine += "<span style=\"color:#666666;\">";
            commentedLine += commentSeparatedStrings[1];
            commentedLine += "</span>";

            line = std::move(commentedLine);
        }
    }
}

void SrcHighlighter::handleMultiLineComments()
{
    bool iterInComment = false; // kommenten belül vagyunk-e?

    for (auto& line : file)
    {
        int n = static_cast<int>(line.size());
        for (int i = 0; i < n - 1; ++i)
        {
            if (!iterInComment)
            {
                if(line[i] == '/' && line[i + 1] == '*')
                {
                    line[i] = line[i + 1] = '~';
                    iterInComment = true;

                    // Megelőző nyitó HTML tagek kiszűrése. (Ha befér egyáltalán)
                    if (i - HTML_OPEN_TAG_LENGTH > 0)
                    {
                        for (int j = 0; i - j >= 0; j++)
                        {
                            if (line.substr(i - j, 5) == "<span" )
                            {
                                for (int k = 0; k < HTML_OPEN_TAG_LENGTH; ++k)
                                {
                                    line[i - j + k] = '@';
                                }
                            }
                            // Az első komment előtti tag záró tag volt, ezzel nincs baj
                            else if (line.substr(i - j, 6) == "</span")
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                if (line[i] == '*' && line[i + 1] == '/')
                {
                    line[i] = line[i + 1] = '`';
                    iterInComment = false;

                    if (i + HTML_CLOSE_TAG_LENGTH < n)
                    {
                        for (int j = i + 2; j < n - HTML_CLOSE_TAG_LENGTH; ++j)
                        {
                            if (line.substr(j, HTML_CLOSE_TAG_LENGTH) == "</span")
                            {
                                for (int k = 0;  k < HTML_CLOSE_TAG_LENGTH; ++k)
                                {
                                    line[j + k] = '@';
                                }
                            }
                            // a komment zárása után nyitó taget teláltunk legelőször, ez nem gáz.
                            else if (line.substr(j, 5) == "<span")
                            {
                                break;
                            }
                        }
                    }
                }
                // komment-en belül vagyunk ténylegesen, a feladat a html tagek eltüntetése
                else
                {
                    if (line.substr(i, 5) == "<span")
                    {
                        for (int j = 0; j < HTML_OPEN_TAG_LENGTH; ++j)
                        {
                            line[i + j] = '@';
                        }
                    }
                    else if (line.substr(i, 6) == "</span")
                    {
                        for (int j = 0; j < HTML_CLOSE_TAG_LENGTH; ++j)
                        {
                            line[i + j] = '@';
                        }
                    }
                }
            }
        }

        line = std::regex_replace(line, std::regex("~~"), R"(<span style="color:#666666;">/*)");
        line = std::regex_replace(line, std::regex("``"), R"(*/</span>)");
        line = std::regex_replace(line, std::regex("@"), "");
    }


}

void SrcHighlighter::doubleCheckStringLiterals()
{
    std::regex regexForBadString(R"(<span style="color:#339900;">".*</span>.*"</span>)");
    for (auto& line : file)
    {
        if (std::regex_search(line, regexForBadString))
        {
            int loc = line.find(R"(<span style="color:#339900;">")");

            int innerNonClosedTags = 0;
            int substitutionOn = false;

            int n = static_cast<int>(line.size());
            for (int i = loc; i < n; ++i)
            {
                if (i + HTML_OPEN_TAG_LENGTH < n && line.substr(i, HTML_OPEN_TAG_LENGTH) == R"(<span style="color:#339900;">)")
                {
                    i += HTML_OPEN_TAG_LENGTH;
                    innerNonClosedTags++;
                    substitutionOn = true;
                }
                else if (i + HTML_OPEN_TAG_LENGTH < n && line.substr(i, 5) == "<span" && substitutionOn)
                {
                    innerNonClosedTags++;
                    for (int j = 0; j < HTML_OPEN_TAG_LENGTH; ++j)
                    {
                        line[i + j] = '`';
                    }

                    i += HTML_OPEN_TAG_LENGTH;
                }
                else if (i + HTML_CLOSE_TAG_LENGTH < n && line.substr(i, HTML_CLOSE_TAG_LENGTH) == "</span>" && innerNonClosedTags > 0 && substitutionOn)
                {
                    innerNonClosedTags--;

                    if (innerNonClosedTags == 0)
                    {
                        substitutionOn = false;
                    }
                    else
                    {
                        for (int j = 0; j < HTML_CLOSE_TAG_LENGTH; ++j)
                        {
                            line[i + j] = '`';
                        }
                    }
                }
            }

            line = std::regex_replace(line, std::regex("`"), "");
        }
    }
}






