/**
 * Program pomocniczy do gry "SameGame" który wykonuje jedno polecenie 
 * usunięcia klocków wydane przez gracza. 
 *
 * Program zaliczeniowy do zajęć laboratoryjnych ze Wstępu do programowania.
 * 
 * Kompilacja: gcc @opcje -zadanie2.c -o zadanie2
 * Wywoływanie: ./zadanie2 `wiersz` `kolumna`
 * Przy wywoływaniu, `wiersz` to liczba całkowita liczona od zera reprezentująca 
 * numer wiersza ruchu a `kolumna`, kolumny ruchu. 
 * 
 * Za pomocą komend -DWIERSZE=n, -DKOLUMNY=m gdzie m,n > 0 można zmienić rozmiar planszy.
 * Za pomocą komendy -DRODZAJE=o gdzie o > 0 można zmienić liczbę rodzajów klocków w grze.
 * 
 * Kompilacja z dodatkowymi komendami: gcc @opcje -DWIERSZE=n -DKOLUMNY=m -DRODZAJE=o zadanie2.c -o zadanie2
 *  
 * Program wczytuje aktualny stan planszy i ruch gracza. 
 * Sprawdza czy klocek który gracz wybrał należy do grupy (przyjamniej liczący dwa klocki do 
 * których można się dostać poprzez ruch o jeden klocek do góry, dołu, w lewo lub w prawo). 
 * Jeśli klocek należy do grupy, cała grupa jest usuwana. 
 * Klocki które w wyniku tego są powyżej pustych pól są odpowiednio przesuwane w dół.
 * Jeśli w wyniku tego jest conajmniej jedna pusta kolumna, to wszystkie niepuste kolumny po jej prawej są 
 * przesuwane w lewo aby pustych kolumn nie było od lewej strony. 
 * Program pokazuje wynikowy stan planszy. 
 * 
 * autor: Natalia Junkiert
 * data: 9 XII 2022
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Definiowanie funkcji MAX i MIN które identyfikują odpowiednio większą i mniejszą liczbę z pary
*/

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * Rozmiar planszy i liczba rodzajów klocków które mogą tworzyć grupę 
 * jeśli nie zostały one zdefiniowane inaczej przez gracza. 
 * 10 jednostek `.` w poziomie.
 * 15 jednostek `.` w pionie.
 * 4 rodzaje klocków (tzn. `0`, `1`, `2`, `3`).
*/

#ifndef WIERSZE
#define WIERSZE 10
#endif

#ifndef KOLUMNY
#define KOLUMNY 15
#endif

#ifndef RODZAJE
#define RODZAJE 4
#endif

/**
 * Funkcja wczytująca dane z wejścia do dwuwymiarowej tablicy typu char.    
*/

void wczytywanie(char plansza[][KOLUMNY]) {
    int znak = 0;

    for (int i = 0; i < WIERSZE; i++) {
        for (int j = 0; j < KOLUMNY; j++)
            plansza[i][j] = (char)getchar();
        znak = getchar();
    }
    znak--;
}

/**
 * Pokazuje stan planszy. 
*/

void pokaz_plansze(char plansza[][KOLUMNY]){
    for(int i = 0; i < WIERSZE; i++){
        for(int j = 0; j < KOLUMNY; j++)
            printf("%c", plansza[i][j]);
        printf("\n");
    }
}

/**
 * Sprawdza czy na polu o współrzędnych określonych argumentami programu, jest klocek należący do grupy mającej co najmniej dwa elementy.
*/

bool czy_grupa(char plansza[][KOLUMNY], int wiersz, int kolumna, char rodzaj_grupy, int dx[4], int dy[4]) {

    if(rodzaj_grupy != '.'){
        for(int i = 0; i < 4; i++)
            if((wiersz + dx[i] >= 0) && (wiersz + dx[i] < WIERSZE) &&
                (kolumna + dy[i] >= 0) && (kolumna + dy[i] < KOLUMNY) &&
                (plansza[wiersz + dx[i]][kolumna + dy[i]] == rodzaj_grupy))
                return true;
    }

    return false;
}

/**
 * Wywołane jeśli grupa została zidentyfikowana. 
 * Rekurencyjnie usuwa klocek poprzez zastąpienie go `.`, sprawdza czy klocek bezpośrednio po lewej, prawej, dole i 
 * górze klocka rozpatrywanego jest klocek takiego samego rodzaju. Jeśli tak, funkcja jest ponownie wywoływana. 
 * Kończy się jak już nie ma więcej klocków tego samego rodzaju do którego można dotrzeć poprzez ruch o klocek 
 * do góry, do dołu, w lewo, lub w prawo.
 * Zapisuje też indeks największego wiersza, oraz indeks najmniejszej i największej kolumny których grupa sięga.   
*/

void usun_grupe(char plansza[][KOLUMNY], int wiersz, int kolumna, char rodzaj_grupy,
    int dx[4], int dy[4], int *min_x, int *max_x, int *max_y) {

    plansza[wiersz][kolumna] = '.';

    for(int i = 0; i < 4; i++) {
        if((wiersz + dx[i] >= 0) && (wiersz + dx[i] < WIERSZE) &&
                (kolumna + dy[i] >= 0) && (kolumna + dy[i] < KOLUMNY) &&
                (plansza[wiersz + dx[i]][kolumna + dy[i]] == rodzaj_grupy)){

                *min_x = MIN((int)*min_x, kolumna + dy[i]);
                *max_x = MAX((int)*max_x, kolumna + dy[i]);
                *max_y = MAX((int)*max_y, wiersz + dx[i]);

                usun_grupe (plansza, wiersz + dx[i], kolumna + dy[i], rodzaj_grupy, dx, dy, min_x, max_x, max_y);
        }
    }
}

/**
 * Wywołane po tym jak grupa klocków została usunięta.
 * Zlicza puste pola w pionie w przedziale [0, max_y] w każdej kolumnie z przedziału [min_x, max_x]. 
 * Odpowiednio wpisuje wartość niepustego pola w miejsce pustego jeśli to niepuste pole znajdowało się bezpośrednio nad pustym,
 * i zamienia pole tej wartości na puste. 
 * W wyniku tego nie ma niepustego pole bezpośrednio nad pustym.
*/

void przesun_w_dol(char plansza[][KOLUMNY], int min_x, int max_x, int max_y){

    for(int i = min_x; i <= max_x; i++){

        int puste = 0;

        for(int j = max_y; j >= 0; j--){
            if (plansza[j][i] == '.'){
                puste += 1;
            } else if (puste != 0){
                plansza[j + puste][i] = plansza[j][i];
                plansza[j][i] = '.';
            }
        }
    }
}

/**
 * Wywołane po tym jak grupa klocków została usunięta.
 * Sprawdza czy w wyniku ruchu pojawiła się conajmniej jedna pusta kolumna, czyli sprawdza przediał [min_x, KOLUMNY - 1].  
 * Jeśli jest pusta kolumna, to odpowiednio "przesuwa" wszystkie niepuste kolumny po prawej od tego miejsca w lewo o liczbę pustych kolumn.
 * W wyniku tego, nie ma pustej kolumny bezpośrednio po lewej od niepustej.  
*/ 

void przesun_w_lewo(char plansza[][KOLUMNY], int min_x){

    int pusta_kolumna = 0;

    for(int i = min_x; i < KOLUMNY; i++){
        if(plansza[WIERSZE - 1][i] == '.'){
            pusta_kolumna += 1;
        } else if (pusta_kolumna != 0) {
            for(int j = WIERSZE - 1; j >= 0; j--){
                if(plansza[j][i] != '.') {
                    plansza[j][i - pusta_kolumna] = plansza[j][i];
                    plansza[j][i] = '.';
                }
            }
        }
    }
}

/**
 * Przeprowadza grę na planszy `plansza`.
 * Wywołuje `wczytywanie`, sprawdza czy gracz wybrał grupę, jeśli tak, wywołuje kolejno `usun_grupe`, `przesun_w_dol` i `przesun_w_lewo`. 
 * Pokazuje nowy stan planszy. 
*/

void gra(int wiersz, int kolumna){
    char plansza[WIERSZE][KOLUMNY];
    wczytywanie(plansza);

    char rodzaj_grupy = plansza[wiersz][kolumna];

    int dx[4] = {0, 0, 1, -1};
    int dy[4] = {1, -1, 0, 0};

    int min_x = kolumna;
    int max_x = kolumna;
    int max_y = wiersz;

    if(czy_grupa(plansza, wiersz, kolumna, rodzaj_grupy, dx, dy)){
        usun_grupe(plansza, wiersz, kolumna, rodzaj_grupy, dx, dy, &min_x, &max_x, &max_y);  
        przesun_w_dol(plansza, min_x, max_x, max_y);
        przesun_w_lewo(plansza, min_x);
    }

    pokaz_plansze(plansza);
}

/**
 * Wczytuje 'ruch' gracza który jest podawany jako agrumenty z którymi wywołano program. 
 * Ruch gracza jest zapisywany w zmiennych `wiersz` i `kolumna`. 
 * Uruchamia grę.
*/

int main(int argc, char  *argv[]) {
    assert(argc == 3);
    const int wiersz = atoi(argv[1]);
    const int kolumna = atoi(argv[2]);
    gra(wiersz, kolumna);
    return 0;
}
