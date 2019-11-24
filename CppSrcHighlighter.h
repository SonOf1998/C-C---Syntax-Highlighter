#ifndef CPPHF2_CPPSRCHIGHLIGHTER_H
#define CPPHF2_CPPSRCHIGHLIGHTER_H

#include <regex>

#include "CSrcHighlighter.h"

#define CPP_KEYWORDS_TXT_PATH "cpp_keywords.txt";

class CppSrcHighlighter final : public CSrcHighlighter
{
    [[nodiscard]] const char* getKeywordsTxtPath() const noexcept override;

    /* Minden soron végigmegyünk és a megfelelő regex-re illeszkedő szövegrészeletek köré
     * türkizkék színt jelentő tageket teszünk.
     */
    void highlightNamespaces();

    /* Végigiterálunk az eredeti fájl összes során.
     * Olyan sorokat keresünk amelyek biztos nem includok és <szöveg> regex-re illeszkednek.
     * A regex-re illeszekdő szöveg elé teszünk egy ~-t, utána meg egy `-t.
     *
     * ~ kicseréljük a nyitó tagre + a < karakter feloldójeles változatára.
     * ` kicseréljük a > feloldójeles változatára és a záró tagre.
     *
     * a <...>-t nem lehet a HTML-ben hagyni, ezért szükséges a < kicserélése &lt-re illetve a > kicserélése &gt-re.     *
     */
    void highlightTemplateParams();


    void highlightProgrammingLangSpecificStuff() override;

public:

    using CSrcHighlighter::CSrcHighlighter;

};


#endif
