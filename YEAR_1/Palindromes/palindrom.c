/**
 * "Connect Four - Palindromy"
 *
 * Program zaliczeniowy do zajęć laboratoryjnych ze Wstępu do programowania.
 *
 * gcc @opcje palindrom.c -o palindrom
 *
 * Za pomocą komend -DWIERSZE=n, -DKOLUMNY=m gdzie m,n należą do przedziału [1, 26] można zmienić rozmiar planszy.
 * Za pomocą komendy -DDLUGOSC=o gdzie o należy do przedziału [1, 26] można zmienić długość palindromu szukanego.
 *
 * gcc @opcje -DWIERSZE=n -DKOLUMNY=m -DDLUGOSC=o palindrom.c -o palindrom
 *
 * Program umożliwia grę w Connect Four - Palindromy
 *
 * Pokazuje który gracz ma zacząć grę.
 * Wczytuje ruch gracza wpisywany literami na klawiaturze.
 * Program pokazuje aktualny stan planszy.
 * Program/gra się kończy wciśnięciem `.` lub wygraną jednego gracza, czyli ułożeniem palindromu o długości DLUGOSC.
 *
 * autor: Natalia Junkiert
 * data: 28 XI 2022
*/

#include <stdio.h>
#include <stdbool.h>

/**
 * Rozmiar planszy jeśli nie jest zdefiniowany przez gracza
 * 8 jednostek ` -` w pionie
 * 8 jednostek ` -` w poziomie
*/

#ifndef WIERSZE
#define WIERSZE 8
#endif

#ifndef KOLUMNY
#define KOLUMNY 8
#endif

/**
 * Długość palindromu który jest szukany (jeśli nie jest zdefiniowany inaczej przez gracza)
*/

#ifndef DLUGOSC
#define DLUGOSC 5
#endif

/**
 * Pokazuje planszę
*/

void pokaz_plansze(int plansza[][KOLUMNY]) {

    for (int i = 0; i < WIERSZE; i++) {
        for (int j = 0; j < KOLUMNY; j++)
            if (plansza[i][j] == 0)
                printf(" -");
            else
                printf(" %d", plansza[i][j]);

        printf("\n");
    }

    for (int i = 0; i < KOLUMNY; i++)
        printf(" %c", 'a' + i);

    printf("\n");
}

bool czy_palindrom(int kolumna, int plansza[][KOLUMNY], int wiersz) {

    /**
     * Sprawdzanie czy jest palindrom w poziomie
     * Przedział kolumn: [kolumna - DLUGOSC + 1, kolumna + DLUGOSC - 1]
    */

    int prawy_indeks = kolumna;
    int lewy_indeks = kolumna - DLUGOSC + 1;

    while (kolumna >= lewy_indeks && kolumna <= prawy_indeks && prawy_indeks < KOLUMNY) {
        int t = 0;

        if (lewy_indeks >= 0)
            while (lewy_indeks + t < prawy_indeks - t && plansza[wiersz][lewy_indeks + t] == plansza[wiersz][prawy_indeks - t] &&
            plansza[wiersz][lewy_indeks + t] != 0 && plansza[wiersz][prawy_indeks - t] != 0)
                t += 1;

        if (lewy_indeks + t >= prawy_indeks - t && plansza[wiersz][lewy_indeks + t] != 0)
            return true;

        lewy_indeks += 1;
        prawy_indeks += 1;
    }

    /**
     * Sprawdzanie czy jest palindrom w pionie
     * Przedział wierszy: [wiersz, wiersz + DLUGOSC - 1]
    */

    if (wiersz + DLUGOSC < WIERSZE + 1) {
        bool palindrom = true;

        for (int d = 0; d < DLUGOSC / 2 + 1; d++)
            if (plansza[wiersz + d][kolumna] != plansza[wiersz + DLUGOSC - d - 1][kolumna])
                palindrom = false;

        if (palindrom == true)
            return true;
    }

    /**
     * Sprawdzanie czy jest palindrom na przekątnych
    */

    int x_lewy = kolumna - DLUGOSC + 1;
    int y_lewy1 = wiersz + DLUGOSC - 1;
    int y_lewy2 = wiersz - DLUGOSC + 1;
    int x_prawy = kolumna;
    int y_prawy1 = wiersz;
    int y_prawy2 = wiersz;

    while (kolumna >= x_lewy && kolumna <= x_prawy && x_prawy < KOLUMNY) {

        /**
         * Sprawdza czy jest palindrom na przekątnej od lewego-dolnego rogu do prawego-górnego
         * Przedział wierszy: [kolumna - DLUGOSC + 1, kolumn + DLUGOSC -1]
         * Przedział kolumn: [wiersz - DLUGOSC + 1, wiersz + DLUGOSC - 1]
        */

        int t = 0;

        if (x_lewy >= 0 && y_lewy1 < WIERSZE)
            while (y_prawy1 >= 0 && plansza[y_lewy1 - t][x_lewy + t] == plansza[y_prawy1 + t][x_prawy - t] &&
            plansza[y_lewy1 - t][x_lewy + t] != 0 && x_lewy + t < x_prawy - t)
                t += 1;

        if (x_lewy + t >= x_prawy - t && plansza[y_prawy1 + t][x_prawy - t] != 0)
            return true;

        t = 0;

        /**
         * Sprawdza czy jest jakiś palindrom na przekątnej od lewego-górnego rogu do prawego-dolnego
         * Przedział wierszy: [kolumna - DLUGOSC + 1, kolumn + DLUGOSC -1]
         * Przedział kolumn: [wiersz - DLUGOSC + 1, wiersz + DLUGOSC - 1]
        */

        if (x_lewy >= 0 && y_lewy2 >= 0)
            while (y_prawy2 < WIERSZE && plansza[y_lewy2 + t][x_lewy + t] == plansza[y_prawy2 - t][x_prawy - t] &&
            plansza[y_lewy2 + t][x_lewy + t] != 0 && x_lewy + t < x_prawy - t)
                t += 1;

        if (x_lewy + t >= x_prawy - t && plansza[y_prawy2 - t][x_prawy - t] != 0)
            return true;

        x_lewy += 1;
        x_prawy += 1;
        y_lewy1 -= 1;
        y_prawy1 -= 1;
        y_lewy2 += 1;
        y_prawy2 += 1;
    }
    return false;
}

/**
 * Przeprowadza grę na planszy `plansza`.
 * Wypisuje numer gracza, wczytuje ruch gracza, pokazuje zmienioną planszę i sprawdza czy gracz wygrał lub gra została zakończona.
*/

void gra(int plansza[][KOLUMNY], int wypelnienie[KOLUMNY]){
    int ruch = '0';
    bool wygrana = false;
    int numer_gracza = 2;

    while (ruch != '.' && wygrana == false) {
        numer_gracza = 3 - numer_gracza;

        pokaz_plansze(plansza);
        printf("%d:\n", numer_gracza);

        ruch = (char)getchar();

        while ((ruch != EOF) && (ruch != '.') && (ruch != '\n')) {

            plansza[WIERSZE - 1 - wypelnienie[(int)(ruch - 'a')]][(int)(ruch - 'a')] = numer_gracza;
            wypelnienie[(int)(ruch - 'a')] += 1;

            wygrana = czy_palindrom((int)(ruch - 'a'), plansza, WIERSZE - wypelnienie[(int)(ruch - 'a')]);

            while ((ruch != '\n') && (ruch != EOF))
                ruch = (char)getchar();
        }
    }

    if (wygrana == true) {
        pokaz_plansze(plansza);
        printf("%d!\n", numer_gracza);
    }
}

/**
 * Uruchamia grę.
 * `wypelnienie` zapisuje ile wierszy w każdej kolumnie jest wypełnionych żetonami.
*/

int main(void) {
    int plansza[WIERSZE][KOLUMNY] = {0};
    int wypelnienie[KOLUMNY] = {0};
    gra(plansza, wypelnienie);
    return 0;
}
