
#include "telefonkonyv.h"

int main()
{
    szemely* szemelyek_lista=NULL;

    szemelyek_lista=lista_feltolt(szemelyek_lista);

    int menupont;
    fomenu_kiir(&menupont);
    int valasztas;
    valasztas_beolvas(&valasztas);

    while(valasztas!=0){
        if(menupont==0 && valasztas==0){
            valasztas=0;
        }

        if(menupont==0 && valasztas==1){
            menu1_kiir(&menupont);
            valasztas_beolvas(&valasztas);
            continue;
        }

        if(menupont==0 && valasztas==2){
            menu2_kiir(&menupont);
            valasztas_beolvas(&valasztas);
            continue;
        }

        if(menupont==0 && valasztas==3){
            lista_kiir(szemelyek_lista);
            fomenu_init(&menupont, &valasztas);
            continue;
        }

        if(menupont==0 && valasztas==4){
            fajlba_kiir(szemelyek_lista);
            printf("Fajlba iras sikeres.");
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==0 && valasztas==5){
            printf("vCard fajlba iras.\n");
            printf("A keresett szemely neve:");
            char* keresett_nev= dyn_string_alloc();
            vcard_kiir(szemelyek_lista, keresett_nev);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==0 && valasztas==6){
            vCard_beolvas(szemelyek_lista);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==1 && valasztas==0){
            valasztas=0;
        }

        if(menupont==1 && valasztas==1){

            szemely* uj_szemely = (szemely*)malloc(sizeof(szemely));
            printf("Uj szemely felvetele\n");
            printf("Nev:");
            (*uj_szemely).nev=dyn_string_alloc();
            printf("Foglalkozas:");
            (*uj_szemely).foglalkozas=dyn_string_alloc();
            printf("Cim:");
            (*uj_szemely).cim=dyn_string_alloc();
            printf("Telefonszam:");
            (*uj_szemely).telefonszam=dyn_string_alloc();

			szemely_hozzaad(&szemelyek_lista, uj_szemely);

            printf("Folytatja? (i/n)\n");
            char c;
            scanf("%c", &c);
            if(c=='n'){
                fomenu_init(&menupont, &valasztas);
            }
        }

        if(menupont==1 && valasztas==2){
            printf("Meglevo szemely adatainak modositasa\n");
            printf("Keresett szemely neve:");
            char* keresett_nev=dyn_string_alloc();
            printf("\nMit kivan modositani?\n");
            printf("[1] Nev\n");
            printf("[2] Foglalkozas\n");
            printf("[3] Cim\n");
            printf("[4] Telefonszam\n");
            int valasztas1;
            scanf("%d", &valasztas1);
            szemelyek_lista=modosit(szemelyek_lista, keresett_nev, valasztas1);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==1 && valasztas==3){
            printf("Meglevo szemely torlese.\n");
            printf("Irja be a torlendo szemely nevet:");
            char* torlendo_nev=dyn_string_alloc();
            szemelyek_lista=torles(szemelyek_lista, torlendo_nev);
            free(torlendo_nev);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==1 && valasztas==4){
            fomenu_kiir(&menupont);
            valasztas_beolvas(&valasztas);
            continue;
        }

        if(menupont==2 && valasztas==0){
            valasztas=0;
        }

        if(menupont==2 && valasztas==1){
            printf("Kereses nev alapjan.\n");
            char parameter[]="nev";
            printf("Irja be a nevet:");
            char* keresett_nev=dyn_string_alloc();
            printf("A keresett szemely adatai:\n");
            if(strchr(keresett_nev, '*') == NULL){
                kereses(szemelyek_lista, keresett_nev, parameter);
            }
            else{
                csillagos_kereses(szemelyek_lista, keresett_nev);
            }

            free(keresett_nev);
            fomenu_init(&menupont, &valasztas);
		}

        if(menupont==2 && valasztas==2){
            printf("Kereses telefonszam alapjan.\n");
            char parameter[]="telefonszam";
            printf("Irja be a telefonszamot (+36xx1234567:");
            char* keresett_telefonszam=dyn_string_alloc();
            printf("A keresett szemely adatai:\n");
            kereses(szemelyek_lista, keresett_telefonszam, parameter);
            free(keresett_telefonszam);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==2 && valasztas==3){
            printf("Kereses foglalkozas alapjan.\n");
            char parameter[]="foglalkozas";
            printf("Irja be a keresett szemely foglalkozasat:");
            char* keresett_foglalkozas=dyn_string_alloc();
            printf("A keresett szemely adatai:\n");
            kereses(szemelyek_lista, keresett_foglalkozas, parameter);
            free(keresett_foglalkozas);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==2 && valasztas==4){
            printf("Kereses cim alapjan.\n");
            char parameter[]="cim";
            printf("Irja be a keresett szemely cimet:");
            char* keresett_cim=dyn_string_alloc();
            printf("A keresett szemely adatai:\n");
            kereses(szemelyek_lista, keresett_cim, parameter);
            free(keresett_cim);
            fomenu_init(&menupont, &valasztas);
        }

        if(menupont==2 && valasztas==5){
            fomenu_kiir(&menupont);
            valasztas_beolvas(&valasztas);
        }

        if(menupont==2 && valasztas==0){
            valasztas=0;
        }

        if(menupont==3){
            valasztas=0;
            continue;
        }

    }
    printf("Viszontlatasra!");
    return 0;
}
