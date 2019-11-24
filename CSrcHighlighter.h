#ifndef CPPHF2_CSRCHIGHLIGHTER_H
#define CPPHF2_CSRCHIGHLIGHTER_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#define C_KEYWORDS_TXT_PATH "c_keywords.txt"
#define PREPROCESSOR_FILE_PATH "preprocessor.txt"

constexpr int HTML_OPEN_TAG_LENGTH = 29;
constexpr int HTML_CLOSE_TAG_LENGTH = 7;

class CSrcHighlighter
{
    std::vector<std::string> keywords;
    std::vector<std::string> prep_dirs;

    [[nodiscard]] virtual const char* getKeywordsTxtPath() const noexcept;

    void importPreprocessorDirectives();
    virtual void importKeywords();

    /* C++ forráskódokra még a namespace-ket
     * és a template paramétereket terveztem megcsinálni.
     *
     * A C++ highlighter ennek az osztálynak egy leszármazottja lesz.
     */
    virtual void highlightProgrammingLangSpecificStuff() {/* üres virtuális */};

    /* Megkeresi az összes stringet és karaktert és zöld színt ad nekik.
     * Az idézőzejeles includokat figyelmen kívül hagyja.
     */
    void highlightCharsAndStringLiterals();

    /* Végignézi az összes sort, és keresi a preprocesszor direktivákat.
     * Ha talál egy ilyet megnézi, hogy van e benne kacsacsőrös include. Ha van akkor a kacsacsőröket escape-eli,
     * mert különben HTML tagnak venné a böngésző.
     *
     * Ellátja az #-es utasítást egy barna színt adó taggel
     * A preprocesszor direktítva "paraméterének" pedig zöld taget ad.
     *
     */
    void highlightPreprocessorDirectives();

    /* Megnézi, hogy az adott sorban-e van e olyan sztring, amelyben numerikus konstans van.
     * Ha van, akkor nem csinál semmit az adott sorral.
     * Ez egy elég rossz stratégia, mert pl. egy stringet és int-et váró függvény hívása nem jelenne meg helyesen.
     * Viszont így legalább regexekkel kezelhető.
     *
     * Tehát ha nincs ilyen a sorban, akkor megkeressük a numerikus konstansokat. Ezt nem lehet olyan simán
     * mert pl. a vltozónevekben lévő számokat is megtalálnánk. Ezért olyan számokat keresek, amelyet kacsaszőr, vessző
     * pontosvessző, zárójelek, egyenlőségjel +/-* stb határol. Egy ilyen tömb köré teszek egy ~ és ` jelet
     * regex replace-szel.
     *
     * Majd bejárom ezeket a ~...szám...` blokkokat. Ha számot találok benne akkor a szám elé beszúrok egy @ jelet.
     * A szám után pedig egy X jelet.
     *
     * Majd az összes @ jelet kicserélem a kék szín nyitó tagjére. Az X-eket pedig záró tagre.
     *
     */
    void highlightNumericConstants();

    /* Végigmegyünk az össze keywordon, benne pedig az összes soron.
     * Megnézzük, hogy a sorban benne van egyáltalán a keyword.
     * Ha benne van, akkor írunk egy regexet, ami megtalálja ezeket. (keywordök lehet space-k tabulátorok, zárójelek,
     * stb. között). Majd az így talált blokkokat egy ~ és ` közé tesszük. Ezeken mint az előző esetben végigmegyünk
     * és a keyword tényleges karaktereit kicseréljük @-re. Majd a keyword hosszúságú @ blokkokat kicseréljük
     * a tagekkel körülhatárolt keyword-re.
     *
     */
    void highlightKeywords();

    /* Olyan sztring literálokat keresünk amelyek hibásak, azaz van bennük HTML tag.
     * (pl keyword volt egy stringben)
     *
     * Megkeressük a hibás string literált. Majd ha találunk benne egy HTML taget akkor annak karaktereit kicseréljük
     * `-re.
     *
     * Az eljárás végén regex_replaccel eltváolítjuk az összes `-t.
     *
     */
    void doubleCheckStringLiterals();


    /* Megkeressük a "-el határolt \t \n \r jelüléseket. Ezeket kivastagítjuk.
     * Ez ilyenekre hibás eredményt adna: "..." \t "...". De ennek az előfordulása nem túl jellemző.
     */
    void highlightEscapes();


    /* Megnézzük, hogy hol van az adott sorban először a // sztring, persze ha egyáltalán van.
     * Ha megtaláltuk, akkor vele együtt a kommentált részt levágjuk és csinálunk vele egy regexet.
     * A regex replace-szel kisceréljük a HTML tagekkel ellátott kommentre.
     * Kis trükközéssel utána pedig a kommentből eltávolitjuk a felesleges HTML tageket.
     *  (pl.: keywordök a kommentben stb)
     */
    void handleSingleLineComments();

    /* Megkeressük a / * előfordulását. (és kicseréljük ~~-re)
     * Ha előtte a legelső HTML tag nyitó akkor az elátvolítjuk.
     *
     * Elkezdünk utána haladni a sorokban és a html tageket kicseréljük @ jelre.
     * Ha megtaláltuk a komment végét jelző * /-t akkor megnézzük, hogy utána milyen tag áll.
     * Ha záró akkor eltávolítjuk. A * /-t pedig kicserékük ``-re.
     *
     * Az eljárás végén az összes @ jelet eltávolítjuk regex_replace-szel.
     * A ~~-t kicseréljük <span ... color:szürke>/ * -re
     * A ``-t pedig * /</span>-re.
     *
     */
    void handleMultiLineComments();

    void doSyntaxHighlighting();

protected:

    std::vector<std::string>& file;             // a beolvasott sorok, amelyeket szerkesztünk és végül kiírunk html-be
    std::vector<std::string> unmodifiedFile;    // beolvasott sorok szerkesztetlenül (sok tagnál már kezelhetetlenek a regexek)

public:

    explicit CSrcHighlighter(std::vector<std::string>&) noexcept;
    virtual ~CSrcHighlighter() = default;


    void exportHighlightedSrcCode(const std::string&);
};


#endif
