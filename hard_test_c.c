#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "konzol_grafika.h"
#include "strukturak.h"
#include "debugmalloc.h"


/*void ekezetmentesites(char *szoveg)
{
    // A probléma a multi character constant hibaüzenettel van.
    // Nem minden 2. pozíciót vizsgáltam, hanem a karaktereket párosával, hiszen a magyar ékezetes
    // betûk utf8-ban két karaktert foglalnak, ez indokolta a strcmp használatát az összehasonlításnál

    // A megoldás a következõ lett: A fájl maga lett ékezettelenítve egy C# programmal, mert
    // ott ilyen problémák nincsenek.
}*/

void feldarabolas(Kerdes *k, char *sor)
{
    sscanf(sor, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%s",
           &k->nehezseg,
           k->kerdes_szoveg,
           k->A.kiiras,
           k->B.kiiras,
           k->C.kiiras,
           k->D.kiiras,
           &k->megoldas,
           k->kategoria
    );

    k->A.betujel = 'A';
    k->B.betujel = 'B';
    k->C.betujel = 'C';
    k->D.betujel = 'D';
}

void mentes(Jatekos j)
{
    FILE *ranglista;
    ranglista = fopen("eredmenyek.txt", "r");

    Jatekos eddigi_jatekosok[10];
    char sor[256];

    if(ranglista)
    {
        int i = 0;
        while(fgets(sor, sizeof(sor), ranglista))
        {
            if(sor[0] != '-')
            {
                sscanf(sor, "%[^\t]\t%d\t%d",
                   eddigi_jatekosok[i].nev,
                   &eddigi_jatekosok[i].nyeremeny,
                   &eddigi_jatekosok[i].eltelt_ido
                );
            } else {
                for(int k = i; k < 10; k++)
                {
                    eddigi_jatekosok[k].nev[0] = '\0';
                    eddigi_jatekosok[k].nyeremeny = -1;
                    eddigi_jatekosok[k].eltelt_ido = -1;
                }
                break;
            }

            i++;
        }
    } else {
        perror("HIBA: ");
        exit(0);
    }
    fclose(ranglista);

    for(int i = 0; i < 10; i++)
    {
        if(j.nyeremeny > eddigi_jatekosok[i].nyeremeny || (j.nyeremeny == eddigi_jatekosok[i].nyeremeny && j.eltelt_ido <= eddigi_jatekosok[i].eltelt_ido))
        {
            memmove(&eddigi_jatekosok[i + 1], &eddigi_jatekosok[i], (10 - i - 1) * sizeof(Jatekos));
            eddigi_jatekosok[i] = j;
            break;
        }
    }

    ranglista = fopen("eredmenyek.txt", "w");

    for(int i = 0; i < 10; i++)
    {
        if(eddigi_jatekosok[i].nyeremeny != -1)
        {
            fprintf(ranglista, "%s\t%d\t%d\r\n", eddigi_jatekosok[i].nev, eddigi_jatekosok[i].nyeremeny, eddigi_jatekosok[i].eltelt_ido);
        } else {
            fprintf(ranglista, "-\r\n");
        }
    }

    fclose(ranglista);
}

int hatvanyozas(int alap, int kitevo)
{
    if(kitevo == 0)
    {
        return 1;
    }

    return alap * hatvanyozas(alap, kitevo - 1);
}

char *szam_formazas(int szam)
{
    int szjegyek_szama = floor(log10(szam)) + 1;
    int n = szjegyek_szama + (szjegyek_szama - 1) / 3 + 1;
    char *p = (char*)malloc(n);
    p[n - 1] = '\0';
    int j = 0;
    for(int i = n - 2; i >= 0; i--)
    {
        if((n - 1 - i) % 4 == 0)
        {
            p[i] = ' ';
        } else {
            p[i] = (szam % hatvanyozas(10, j + 1)) / hatvanyozas(10, j) + '0';
            j++;
        }
    }

    return p;
}

void szavazas(int nehezseg, char megoldas, double *a, double *b, double *c, double *d)
{
    double szazalekok[4] = {0};

    double randomertek = ((rand() % (9633 - 9325 + 1) + 9325) - (nehezseg * (rand() % (533 - 525 + 1) + 525))) / 100.0;
    szazalekok[megoldas - 'A'] = randomertek;

    double osszeg = szazalekok[0] + szazalekok[1] + szazalekok[2] + szazalekok[3];
    while(osszeg != 100.0)
    {
        for(int i = 0; i < 4; i++)
        {
            if(szazalekok[i] != randomertek)
            {
                szazalekok[i] = (rand() % 3001) / 100.0;
            }
        }
        osszeg = szazalekok[0] + szazalekok[1] + szazalekok[2] + szazalekok[3];
    }

    *a = szazalekok[0];
    *b = szazalekok[1];
    *c = szazalekok[2];
    *d = szazalekok[3];
}

int fixnyeremeny(int eddigi_nyeremeny)
{
    //Akkor hívódik meg ha a játékos hibásan válaszol a kérdésre
    //Ekkor ugyebár az eddig megszerzett legnagyobb fixnyereményt kapja meg

    if(eddigi_nyeremeny < 10000)
    {
        return 0;
    } else if(eddigi_nyeremeny < 100000) {
        return 10000;
    } else if(eddigi_nyeremeny < 1000000) {
        return 100000;
    } else if(eddigi_nyeremeny < 5000000) {
        return 1000000;
    }

    return 5000000;
}

void hozzafuz(Kerdes **lista, Kerdes kerdes)
{
    Kerdes *uj = (Kerdes*)malloc(sizeof(Kerdes));
    *uj = kerdes;
    uj->kov = *lista;

    if(*lista != NULL)
    {
        Kerdes *iter = *lista;
        while(iter->kov != *lista)
        {
            iter = iter->kov;
        }
        iter->kov = uj;
    } else {
        uj->kov = uj;
    }

    *lista = uj;
}

Kerdes randomkerdes(Kerdes *lista)
{
    int random = rand() % 500;
    Kerdes *iter = lista;

    while(random != 0)
    {
        iter = iter->kov;
        random--;
    }

    return *iter;
}

void felszabadit(Kerdes *lista)
{
    Kerdes *iter = lista;

    while(iter->kov != lista)
    {
        Kerdes *tmp = iter->kov;
        free(iter);
        iter = tmp;
    }
    free(iter);
}

int main()
{
    fomenu_megjelenites();
    char input[2];
    char BEOLVASOTT_KARAKTER;

    // Amennyiben nem létezik az eredmenyek.txt, létrehozzuk és
    // feltöltjük tíz sorát '-' jelekkel. Ez jelenti a még üres ranglistát.
    FILE *ranglista = fopen("eredmenyek.txt", "rb+");
    if(ranglista == NULL)
    {
        fclose(ranglista);
        ranglista = fopen("eredmenyek.txt", "wb");
        for(int i = 0; i < 10; i++)
        {
            fprintf(ranglista, "-\r\n");
        }
    }
    fclose(ranglista);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// Fõmenü input-elágazás blokk  /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    scanf("%2s%*[^\n]", input);
    BEOLVASOTT_KARAKTER = toupper(input[0]);
    while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'J' && BEOLVASOTT_KARAKTER != 'E' && BEOLVASOTT_KARAKTER != 'x'))
    {
        printf("Hibas karakter J/E/x: ");
        scanf("%2s%*[^\n]", input);
        BEOLVASOTT_KARAKTER = toupper(input[0]);
    }

    if(BEOLVASOTT_KARAKTER == 'x')
    {
        printf("\n\n--------------------------------------------\n");
        printf("--- A PROGRAMBOL VALO KILEPES MEGTORTENT ---\n");
        printf("--------------------------------------------\n");
        return 0;
    } else if(BEOLVASOTT_KARAKTER == 'E') {
        eredmenytabla_megjelenites();
        scanf("%2s%*[^\n]", input);
        BEOLVASOTT_KARAKTER = toupper(input[0]);
        while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'J' && BEOLVASOTT_KARAKTER != 'x'))
        {
            printf("Hibas karakter J/x: ");
            scanf("%2s%*[^\n]", input);
            BEOLVASOTT_KARAKTER = toupper(input[0]);
        }

        if(BEOLVASOTT_KARAKTER == 'x')
        {
            printf("\n\n--------------------------------------------\n");
            printf("--- A PROGRAMBOL VALO KILEPES MEGTORTENT ---\n");
            printf("--------------------------------------------\n");
            return 0;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    FILE *szovegfajl = fopen("kerdesek.txt", "r");
    char sor[256];

    Kerdes *kerdes_listak[10] = {NULL};
    bool elso_sor_beolvasva = false;

    if(szovegfajl)
    {
        while (fgets(sor, sizeof(sor), szovegfajl))
        {
            if(!elso_sor_beolvasva)
            {
                // UTF8 (BOM) Byte Order Mark eltávolítás a legelsõ sorból
                // sor[0,1,2] = 0xEF,0xBB,0xBF
		// <J/x/E>

                memmove(sor, sor + 3, sizeof(sor));
                elso_sor_beolvasva = true;
            }

            Kerdes kerdes;
            feldarabolas(&kerdes, sor);
            if(kerdes.nehezseg <= 10)
            {
                hozzafuz(&kerdes_listak[kerdes.nehezseg - 1], kerdes);
            } else {
                hozzafuz(&kerdes_listak[9], kerdes);
            }
        }
    } else {
        perror("HIBA: ");
        return 0;
    }

    fclose(szovegfajl);

    srand(time(0));

    int const nyeremenyek[10] = {
        10000,      //FIxNYERERMÉNY
        50000,
        100000,     //FIxNYERERMÉNY
        250000,
        500000,
        1000000,    //FIxNYERERMÉNY
        2000000,
        5000000,    //FIxNYERERMÉNY
        10000000,
        20000000
    };


    bool jatekban_vagyunk = true;
    time_t start, vege;

    bool felezes_hasznalhato;
    bool kozonseg_hasznalhato;

    int eddigi_nyeremeny;
    Jatekos jatekos;
    char nev[14];
    do
    {
        //////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////  Név bekérés blokkja  //////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////
        bool nev_megfelelo = false;
        while(!nev_megfelelo)
        {
            nev_bekeres();
            scanf(" %[^\n]s%*[^\n]", nev);

            nev_megfelelo = true;
            int i = 0;
            while(nev[i] != '\0')
            {
                if(!((nev[i] <= 'z' && nev[i] >= 'a') || (nev[i] <= 'Z' && nev[i] >= 'A') || (nev[i] <= '9' && nev[i] >= '0') || nev[i] == ' '))
                {
                    nev_megfelelo = false;
                    break;
                }
                i++;
            }


            if(strlen(nev) > 12)
            {
                nev_megfelelo = false;
            }
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////


        // Ha a megfelelõ név megvan akkor kinullázhatjuk a játékot meghatározó változókat..
        // Az, hogy ezt a do-while cikluson belül tesszük, azért jó, mert így meg tudjuk oldani
        // egyszerûen egy új játék indítását.
        kozonseg_hasznalhato = true;
        felezes_hasznalhato = true;
        eddigi_nyeremeny = 0;
        time(&start);

        for(int i = 0; i < 10; i++)
        {
            Kerdes k = randomkerdes(kerdes_listak[i]);
            kerdes_kiiras(k, i, nyeremenyek[i], felezes_hasznalhato, kozonseg_hasznalhato);
/**/        scanf("%2s%*[^\n]", input);
            BEOLVASOTT_KARAKTER = toupper(input[0]);
            while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'A' && BEOLVASOTT_KARAKTER != 'B' && BEOLVASOTT_KARAKTER != 'C' && BEOLVASOTT_KARAKTER != 'D' &&
            !(BEOLVASOTT_KARAKTER == 'F' && felezes_hasznalhato) && !(BEOLVASOTT_KARAKTER == 'K' && kozonseg_hasznalhato)))
            {
                printf("Ismeretlen karakter! Probalja ujra! A/B/C/D");
                if(felezes_hasznalhato)
                {
                    printf("/F");
                }
                if(kozonseg_hasznalhato)
                {
                    printf("/K");
                }
                printf(">: ");
                scanf("%2s%*[^\n]", input);
                BEOLVASOTT_KARAKTER = toupper(input[0]);
            }

            if(BEOLVASOTT_KARAKTER == k.megoldas)
            {
                if(i == 10 - 1)
                {
                    printf("\nHelyes valasz!\n\n");
                    printf(">>> Gratulalunk a fonyeremenyhez! <<<");

                    eddigi_nyeremeny = nyeremenyek[i];
                    break;
                }
                printf("\nHelyes valasz. Folytassuk? I/N: ");
                scanf("%2s%*[^\n]", input);
                BEOLVASOTT_KARAKTER = toupper(input[0]);
                while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'I' && BEOLVASOTT_KARAKTER != 'N'))
                {
                    printf("Ervenytelen! <I/N>: ");
                    scanf("%2s%*[^\n]", input);
                    BEOLVASOTT_KARAKTER = toupper(input[0]);
                }
                eddigi_nyeremeny = nyeremenyek[i];
                if(BEOLVASOTT_KARAKTER == 'N')
                {
                    break;
                }
            } else if(BEOLVASOTT_KARAKTER == 'K') {
                kozonseg_hasznalhato = false;
                double a, b, c, d;
                szavazas(k.nehezseg, k.megoldas, &a, &b, &c, &d);
                szavazas_kiiras(a, b, c, d);
                printf("\nSzavazas! Valasszon! A/B/C/D: ");
/**/            scanf("%2s%*[^\n]", input);
                BEOLVASOTT_KARAKTER = toupper(input[0]);
                while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'A' && BEOLVASOTT_KARAKTER != 'B' && BEOLVASOTT_KARAKTER != 'C' && BEOLVASOTT_KARAKTER != 'D'))
                {
                    printf("Ismeretlen karakter! Probalja ujra! <A/B/C/D>: ");
                    scanf("%2s%*[^\n]", input);
                    BEOLVASOTT_KARAKTER = toupper(input[0]);
                }

                if(BEOLVASOTT_KARAKTER == k.megoldas)
                {
                    if(i == 10 - 1)
                    {
                        printf("\nHelyes valasz!\n\n");
                        printf(">>> Gratulalunk a fonyeremenyhez! <<<");

                        eddigi_nyeremeny = nyeremenyek[i];
                        break;
                    }
                    printf("\nHelyes valasz. Folytassuk? <I/N>: ");
                    scanf("%2s%*[^\n]", input);
                    BEOLVASOTT_KARAKTER = toupper(input[0]);
                    while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'I' && BEOLVASOTT_KARAKTER != 'N'))
                    {
                        printf("Ervenytelen! <I/N>: ");
                        scanf("%2s%*[^\n]", input);
                        BEOLVASOTT_KARAKTER = toupper(input[0]);
                    }
                    eddigi_nyeremeny = nyeremenyek[i];
                    if(BEOLVASOTT_KARAKTER == 'N')
                    {
                        break;
                    }
                } else {
                    printf("\nHelytelen valasz. A helyes valasz a '%c' volt.", k.megoldas);
                    eddigi_nyeremeny = fixnyeremeny(eddigi_nyeremeny);
                    break;
                }
            } else if(BEOLVASOTT_KARAKTER == 'F') {
                felezes_hasznalhato = false;
                bool valaszthato[4] = {false};
                valaszthato[k.megoldas - 'A'] = true;

                int masik = rand() % 4;
                while(valaszthato[masik] == true)
                {
                    masik = rand() % 4;
                }

                if(masik + 'A' < k.megoldas)
                {
                    printf("\nFelezes! Valasszon! %c/%c: ", masik + 'A', k.megoldas);
                } else {
                    printf("\nFelezes! Valasszon! %c/%c: ", k.megoldas, masik + 'A');
                }

/**/            scanf("%2s%*[^\n]", input);
                BEOLVASOTT_KARAKTER = toupper(input[0]);
                while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != masik + 'A' && BEOLVASOTT_KARAKTER != k.megoldas))
                {
                    if(masik + 'A' < k.megoldas)
                    {
                        printf("Ismeretlen karakter! Probalja ujra! %c/%c: ", masik + 'A', k.megoldas);
                    } else {
                        printf("Ismeretlen karakter! Probalja ujra! %c/%c: ", k.megoldas, masik + 'A');
                    }
                    scanf("%2s%*[^\n]", input);
                    BEOLVASOTT_KARAKTER = toupper(input[0]);
                }

                if(BEOLVASOTT_KARAKTER == k.megoldas)
                {
                    if(i == 10 - 1)
                    {
                        printf("\nHelyes valasz!\n\n");
                        printf(">>> Gratulalunk a fonyeremenyhez! <<<");

                        eddigi_nyeremeny = nyeremenyek[i];
                        break;
                    }
                    printf("\nHelyes valasz. Folytassuk? <I/N>: ");
                    scanf("%2s%*[^\n]", input);
                    BEOLVASOTT_KARAKTER = toupper(input[0]);
                    while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'I' && BEOLVASOTT_KARAKTER != 'N'))
                    {
                        printf("Ervenytelen! I/N: ");
                        scanf("%2s%*[^\n]", input);
                        BEOLVASOTT_KARAKTER = toupper(input[0]);
                    }
                    eddigi_nyeremeny = nyeremenyek[i];
                    if(BEOLVASOTT_KARAKTER == 'N')
                    {
                        break;
                    }
                } else {
                    printf("\nHelytelen valasz. A helyes valasz a '%c' volt.", k.megoldas);
                    eddigi_nyeremeny = fixnyeremeny(eddigi_nyeremeny);
                    break;
                }
            } else {
                printf("\nHelytelen valasz. A helyes valasz a '%c' volt.", k.megoldas);
                eddigi_nyeremeny = fixnyeremeny(eddigi_nyeremeny);
                break;
            }
        }








        time(&vege);
        if(eddigi_nyeremeny != 0)
        {
            strcpy(jatekos.nev, nev);
            jatekos.eltelt_ido = (int)difftime(vege, start);
            jatekos.nyeremeny = eddigi_nyeremeny;

            mentes(jatekos);
        }

        jatekban_vagyunk = false;

        ujjatek_kerdes(eddigi_nyeremeny);
        scanf("%2s%*[^\n]", input);
        BEOLVASOTT_KARAKTER = toupper(input[0]);
        while(strlen(input) != 1 || (BEOLVASOTT_KARAKTER != 'I' && BEOLVASOTT_KARAKTER != 'N'))
        {
            printf("Ervenytelen! <I/N>: ");
            scanf("%2s%*[^\n]", input);
            BEOLVASOTT_KARAKTER = toupper(input[0]);
        }

        if(BEOLVASOTT_KARAKTER == 'I')
        {
            jatekban_vagyunk = true;
        } else {
            printf("\n\n--------------------------------------------\n");
            printf("--- A PROGRAMBOL VALO KILEPES MEGTORTENT ---\n");
            printf("--------------------------------------------\n");
        }

    } while(jatekban_vagyunk);


    // A játék végeztével a kérdések tárolására szolgáló memória felszabadítása
    for(int i = 0; i < 10; i++)
    {
        felszabadit(kerdes_listak[i]);
    }

    return 0;
}


int f(int a, int b)
{  
    if(a < b)
    {
	return a;
    }	

    // :O // :o		
    return (1,2);		
}




