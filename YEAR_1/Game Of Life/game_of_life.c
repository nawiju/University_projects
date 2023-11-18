/**
 * Program umożliwiający grę w "Game of Life"/"Gra w Życie" który wczytuje generację początkową, i wykonuje polecenia
 * gracza.
 * 
 * Program zaliczeniowy do zajęć laboratoryjnych ze Wstępu do programowania.
 * 
 * Kompilacja: gcc @opcje -zadanie3.c -o zadanie3
 * Wywoływanie: ./zadanie3 < generacja.txt
 * Plik generacja.txt ma wypisaną generację początkową oraz polecenia od gracza które kończą się znakiem ".". 
 * 
 * Za pomocą komend -DWIERSZE=n, -DKOLUMNY=m gdzie m,n > 0 można zmienić rozmiar okna.
 * Kompilacja z dodatkowymi komendami: gcc @opcje -DWIERSZE=n -DKOLUMNY=m zadanie3.c -o zadanie3
 * 
 * Program wczytuje początkową generację i polecenia gracza. 
 * Potrafi obliczyć X generacji, przy czym X jest podany przez gracza jako liczba naturalna, różna od zera oraz "\n" 
 * oznacza obliczenie jednej generacji.
 * Polecenie "0" wypisze aktualny stan generacji w tej samej postaci co na wejściu. 
 * Program pokazuje wizualnie aktualny stan komórek które mieszczą się w oknie przed wczytaniem każdej instrukcji.
 * Program się kończy jak wczyta znak ".". 
 * 
 * autor: Natalia Junkiert
 * data: 10 I 2023 
*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <limits.h>

/**
 * Definiowanie funkcji MAX i MIN które identyfikują odpowiednio większą i mniejszą liczbę z pary
 */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * Rozmiar okna jeżeli nie było zdefiniowane inaczej przez użytkownika.
 * 80 jednostek `.` w poziomie.
 * 22 jednostek `.` w pionie.
 */

#ifndef WIERSZE
#define WIERSZE 22
#endif

#ifndef KOLUMNY
#define KOLUMNY 80
#endif

/**
 * Deklaracja typów potrzebnych do struktury danych przechowującą aktualny stan planszy.
 */

typedef struct _branchList list;
typedef struct _rootList rootList;

/**
 * Elementy typu _branchList przechowują wartość kolumn w których znajduje się żywa komórka.
 * list->next i list->prev przechowują odpowiednio wartośc kolumny następnej i poprzedniej żywej komórki
 * w tym wierszu.
 * list->state określa stan tej komórki: 0 oznacza nową komórkę której nie używamy do obliczania nowej generacji;
 * 1 oznacza żywą komórkę której używamy do obliczania nowej generacji; -1 to komórka która niebawem będzie usunięta
 * ale używamy jej do obliczenia nowej generacji; 2 to element atrapy.
 */

struct _branchList {
    int column;
    int state;
    list *prev;
    list *next;
};
/**
 * Elementy typu _rootList przechowują wartość wiersza w którym istnieje przynajmniej jedna żywa komórka.
 * rootList->next i rootList->prev przechowują nody z odpowiednio następnym i poprzednim wierszem w którym
 * jest przynajmniej jedna żywa komórka uporządkowane rosnąco.
 * rootList->right to wskaźnik do _branchList'y.
 * rootList->state określa stan tego wiersza: 0 oznacza nowy wiersz nie używany w obliczeniu nowej generacji;
 * 1 oznacza żywy wiersz używany (z grubsza) do obliczeń; 2 oznacza atrapę.
 */

struct _rootList {
    int row;
    int state;
    list *right;
    rootList *prev;
    rootList *next;
};

/**
 * Funkcja tworząca nowy pusty element na końcu rootList'y przy wczytywaniu danych z wejścia i przypisujący
 * poprzedniemu node'owi wartość wiersza który został wczytany.
 */

rootList *createRootListElement(int row, rootList *head) {
    rootList *newElement = malloc(sizeof(rootList));
    head->next = newElement;
    head->state = 1;
    head->row = row;
    head->right = malloc(sizeof(*head->right));
    head->right->next = NULL;
    head->right->prev = NULL;
    newElement->prev = head;
    newElement->next = NULL;

    return newElement;
}

/**
 * Funkcja tworząća nowy pusty element na końcu listy i przypisująca wartość kolumny wczytywanej do poprzedniego
 * elementu.
*/

list *createBranchListElement(int column, list *leaf) {
    list *newElement = malloc(sizeof(list));
    leaf->column = column;
    leaf->next = newElement;

    newElement->next = NULL;
    newElement->prev = leaf;

    return newElement;
}

/**
 * Funkcja wczytująca wartość kolumny żywych komórek z wejścia i tworząca nowy pusty element na koniec listy
 * wywołując funkcję createBranchListElement() i przypisującą poprzedzającemu node'owi wartość kolumny.
*/

void readingIntoBranchList(rootList *head) {
    list *leaf = head->prev->right;
    leaf->prev = NULL;
    bool inputExists = true;
    char character;

    while (inputExists) {
        while ((character = (char)getchar()) == ' ');

        if (character == '\n') {
            inputExists = false;
        } else {
            ungetc((int)character, stdin);
            int column;
            assert(scanf("%d", &column) > 0);
            leaf->state = 1;
            leaf = createBranchListElement(column, leaf);
        }
    }
    leaf->prev->next = NULL;
    free(leaf);
}

/**
 * Funkcja wczytująca wartość wiersza z wejśćia w której jest conajmniej jedna żywa komórka, i tworząca
 * nowy pusty node na końcu rootList'y oraz przypisująca poprzedniemu elementowi wartość wiersza używając funkcji
 * createRootListElement().
*/

void readingIntoRootList(rootList *head) {
    char character = (char)getchar();
    bool inputExists = true;

    while (inputExists) {
        if (character == '/') {
            character = (char)getchar();
            if (character != '\n') {
                ungetc((int)character, stdin);
                int row;
                scanf("%d", &row);
                head = createRootListElement(row, head);
                readingIntoBranchList(head);
            } else
                inputExists = false;
        }
        character = (char)getchar();
    }

    ungetc(character, stdin);
    head->prev->next = NULL;
    free(head);
}

/**
 * Drukowanie ending - *rootCounter linijek "pustych" (tzn. wypełnionymi tylko ".") na wyjściu.
*/

void printEmptyRow(int *rootCounter, int ending) {
    for (int j = *rootCounter; j < ending; j++) {
        for (int i = 0; i < KOLUMNY; i++)
            printf(".");
        printf("\n");
    }
}

/**
 * Drukowanie x + 1 - *branchCounter "." na wyjściu.
*/

void printEmptyUntilX(int *branchCounter, int x) {
    while (*branchCounter <= x) {
        printf(".");
        (*branchCounter) += 1;
    }
}

/**
 * Drukuje stan wiersza na wyjściu używając "." by reprezentować nieżywą komórkę i "0" jako żywą.
*/

void printRow(rootList *rootPointer, int window) {
    int branchCounter = window;
    list *branchPointer = rootPointer->right;

    while (branchCounter <= window + KOLUMNY - 1) {
        if (branchPointer != NULL && branchPointer->column >= window && branchPointer->column <= window + KOLUMNY - 1) {
            printEmptyUntilX(&branchCounter, branchPointer->column - 1);
            printf("0");
            branchCounter += 1;
            branchPointer = branchPointer->next;
        } else if (branchPointer != NULL && branchPointer->column < window)
            branchPointer = branchPointer->next;
        else
            printEmptyUntilX(&branchCounter, window + KOLUMNY - 1);
    }
    printf("\n");
}

/**
 * Funkcja drukująca aktualny stan okna w zakresie [window[0], window[0] + WIERSZE - 1] w pionie i
 * [window[1], window[1] + KOLUMNY - 1] w poziomie.
 * Wypisuje jeden wiersz "=" na końcu.
*/

void printCurrentState(int window[], rootList *beginning) {
    int rootCounter = window[0];
    rootList *rootPointer = beginning;

    while (rootCounter < window[0] + WIERSZE) {
        if (rootPointer != NULL && rootPointer->row >= window[0] && rootPointer->row <= window[0] + WIERSZE - 1) {
            printEmptyRow(&rootCounter, rootPointer->row);
            printRow(rootPointer, window[1]);
            rootCounter = rootPointer->row + 1;
        } else if ((rootPointer != NULL && rootPointer->row >= window[0]) || rootPointer == NULL) {
            printEmptyRow(&rootCounter, rootCounter + 1);
            rootCounter += 1;
        }

        if (rootPointer != NULL)
            rootPointer = rootPointer->next;
    }

    for (int i = 0; i < KOLUMNY; i++)
        printf("=");
    printf("\n");
}

/**
 * Drukuje wiersze i kolumny w których znajdują się żywe komórki w tym samym formacie co dostaje się dane na wejściu.
*/

void printCoordinates(rootList *beginning) {
    rootList *rootPointer = beginning;

    while (rootPointer != NULL) {
        printf("/%d", rootPointer->row);
        list *branchPointer = rootPointer->right;

        while (branchPointer != NULL) {
            printf(" %d", branchPointer->column);
            branchPointer = branchPointer->next;
        }

        printf("\n");
        rootPointer = rootPointer->next;
    }
    printf("/\n");
}

/**
 * Funkcja tworząca "atrapę" która zawsze ma empty->row = INT_MAX i empty->right->column = INT_MAX.
 * Ten node ma stan (state) równy 2 aby rozróżnić od elementów struktury danych.
*/

rootList *createEmptyNode() {
    rootList *empty = malloc(sizeof(rootList));
    empty->prev = NULL;
    empty->next = NULL;
    empty->right = malloc(sizeof(list));
    empty->right->column = INT_MAX;
    empty->right->next = NULL;
    empty->right->prev = NULL;
    empty->row = INT_MAX;
    empty->state = 2;
    empty->right->state = 2;
    return empty;
}

/**
 * Tworzenie nowego node'a w rootList'cie który przechowuje wartość wiersza który zostanie dodany do struktury.
 * Ten node ma stan (state) równy 0 ponieważ jest nowy.
*/

rootList *createNewRootCell(int rowNumber) {
    rootList *newRow = malloc(sizeof(rootList));
    newRow->row = rowNumber;
    newRow->state = 0;
    newRow->right = NULL;

    return newRow;
}

/**
 * Funkcja dodająca nowy node/wiersz gdzie pojawiły się nowe żywe komórki i odpowiednio zmieniając wskaźniki
 * prev i next dla poprzedzającego i następnego node'a/wiersza.
*/

void addRow(int rowNumber, rootList *pointerAbove, rootList *pointerBelow, rootList **beginning,
            rootList **rootIndex) {
    rootList *newRow = createNewRootCell(rowNumber);

    if (pointerAbove->state == 2) {
        if (pointerBelow->prev != NULL) {
            pointerBelow->prev->next = newRow;
            newRow->prev = pointerBelow->prev;
        } else {
            newRow->prev = NULL;
            *beginning = newRow;
        }
        newRow->next = pointerBelow;
        pointerBelow->prev = newRow;
    } else if (pointerBelow->state == 2) {
        if (pointerAbove->next != NULL) {
            pointerAbove->next->prev = newRow;
            newRow->next = pointerAbove->next;
        } else
            newRow->next = NULL;

        newRow->prev = pointerAbove;
        pointerAbove->next = newRow;
    } else {
        pointerAbove->next = newRow;
        newRow->prev = pointerAbove;
        pointerBelow->prev = newRow;
        newRow->next = pointerBelow;
    }

    *rootIndex = newRow;
}

/**
 * Tworzenie nowego node'a w list'cie który ma stan 0 oraz przechowuje wartość kolumny w której komórka stała
 * się żywa.
*/

list *createNewCell(int column) {
    list *newCell = malloc(sizeof(list));
    newCell->column = column;
    newCell->state = 0;

    newCell->next = NULL;
    newCell->prev = NULL;

    return newCell;
}

/**
 * Funkcja dodająca nowy node który przechowuje wartość kolumny nowej komórki i odpowiednio zmienia wskaźniki
 * prev i next dla poprzedzającego i następnego node'a.
*/

void addCell(int c, list **lastCurrentCell, rootList **rootIndex) {
    list *newCell = createNewCell(c);

    if ((*rootIndex)->right == NULL) {

        (*rootIndex)->right = newCell;
        *lastCurrentCell = newCell;
    } else if ((*lastCurrentCell)->column < c) {

        newCell->next = (*lastCurrentCell)->next;
        (*lastCurrentCell)->next = newCell;
        newCell->prev = (*lastCurrentCell);

        if (newCell->next != NULL)
            newCell->next->prev = newCell;
        *lastCurrentCell = newCell;
    } else if ((*lastCurrentCell)->prev == NULL) {

        (*rootIndex)->right = newCell;
        newCell->next = *lastCurrentCell;
        (*lastCurrentCell)->prev = newCell;
    } else if ((*lastCurrentCell)->column > c) {

        (*lastCurrentCell)->prev->next = newCell;
        newCell->prev = (*lastCurrentCell)->prev;
        (*lastCurrentCell)->prev = newCell;
        newCell->next = *lastCurrentCell;
    }
}

/**
 * Funkcja która zlicza ile jest sąsiadów komórki rozpatrywanej albo w wierszu wyżej bądź niżej w zależności od tego z 
 * jakim wskaźnikiem wywoływana jest funkcja. 
 * Warto zauważyć że te wszystkie warunki nie mogą zachodzić jednocześnie, zatem maksymalną wartością zwracaną przez 
 * funkcję jest 3, czyli maksymalną liczbę sąsiadów. 
*/

int countNeighbours(list *pointer, int column) {
    int neighbours = 0;
    if (pointer != NULL) {
        neighbours += (pointer->state != 0 && pointer->column == column + 1);

        neighbours += (pointer->state != 0 && pointer->column == column);
        neighbours += (pointer->state != 0 && pointer->column == column - 1);

        neighbours += (pointer->prev != NULL && pointer->prev->state != 0 && (pointer->prev->column == column - 1 
                        || pointer->prev->column == column || pointer->prev->column == column + 1));
        neighbours += (pointer->next != NULL && pointer->next->state != 0 && (pointer->next->column == column + 1 
                        || pointer->next->column == column || pointer->next->column == column - 1));
    }
    return neighbours;
}

/**
 * Funkcja która zlicza ile jest sąsiadów rozpatrywanej komórki w jej wierszu (czyli na prawo i lewo). 
 * Warto zauważyć że maksymalnie dwa z tych warunków mogą zachodzić w tym samym momencie. 
*/

int countInLineNeighbours(list *pointer, int c) {
    int neighbours = 0;

    if (pointer != NULL) {
        neighbours += (pointer->state != 0 && pointer->column == c + 1);
        neighbours += (pointer->state != 0 && pointer->column == c - 1);
        neighbours += (pointer->next != NULL && pointer->next->column == c + 1);
        neighbours += (pointer->prev != NULL && (pointer->prev->state != 0 && pointer->prev->column == c - 1));
    }
    return neighbours;
}

/**
 * Zlicza liczbę żwywych sąsiadów rozpatrywanej komórki. Jeśli jest żywa i ma dwóch lub trzech sąsiadów to pozostaje 
 * żywa, czyli ma stan (state) równy 1, w przeciwnym przypadku jest do usunięcia więc jej stan staje się -1. Jeśli 
 * komórka była martwa i ma dokładnie trzech sąsiadów, to zostaje dodawana do struktury przez wywoływanie funkcji
 * addCell(), a jeśli jest ona pierwszą ożywioną w pustym wierszu to najpierw jest on tworzony poprzez wywoływanie 
 * funkcji addRow().
*/

void analyzeCell(list *above, list *below, list *current, rootList *pointerAbove, rootList *pointerBelow,
                 int c, int state, int rowNumber, rootList **beginning, list **lastCurrentCell, rootList **rootIndex) {

    int neighbours = 0;
    neighbours += countNeighbours(above, c);
    neighbours += countNeighbours(below, c);
    neighbours += countInLineNeighbours(current, c);

    if (state == 0 && neighbours == 3) {
        if ((*rootIndex)->state == 2 && (*rootIndex)->row != rowNumber)
            addRow(rowNumber, pointerAbove, pointerBelow, beginning, rootIndex);
        addCell(c, lastCurrentCell, rootIndex);
    } else if (state == 1 && current->state != 0) {
        if (neighbours == 2 || neighbours == 3)
            current->state = 1;
        else
            current->state = -1;
    }
}

/**
 * Funkcja przesuwająca odpowiednie wskaźniki odpowiednio do następnego node'a oraz zwiększająca wartość *column. 
*/

void movePointers(list **above, list **below, list **current, int *column) {
    if ((*above)->next != NULL && ((*column) == (*above)->column || (*above)->state == 2))
        (*above) = (*above)->next;
    if ((*below)->next != NULL && ((*column) == (*below)->column || (*below)->state == 2))
        (*below) = (*below)->next;
    if ((*current) != NULL && ((*column) == (*current)->column || (*current)->state == 2))
        (*current) = (*current)->next;

    (*column) += 1;
}

/**
 * Funkcja wywołująca analyzeCell() dla pustych i żywych komórek w rozpatrywanym wierszu z odpowiednimi argumentami.
 * lastCurrentCell zawsze wskazuje do ostatniej żywej komórki w rozpatrywanym dotychczas zakresie w wierszu rozpatrywanym. Ta 
 * zmienna jest przydatna w addCell(). 
*/

void analyzeNormalCase(list *above, list *below, list *current, rootList *pointerAbove, rootList *pointerBelow,
                       int column, int rowNumber, rootList **beginning, list **lastCurrentCell, rootList **rootIndex) {

    if (current == NULL || (current != NULL && column < current->column)) {
        analyzeCell(above, below, current, pointerAbove, pointerBelow, column, 0,
                    rowNumber, beginning, lastCurrentCell, rootIndex);
    } else {
        if (current != NULL)
            *lastCurrentCell = current;
        analyzeCell(above, below, current, pointerAbove, pointerBelow, column, 1,
                    rowNumber, beginning, lastCurrentCell, rootIndex);
    }
}

/**
 * Funkcja sprawdzająca czy martwa komórka bezpośrednio za ostatnią żywą komórką w wierszu rozpatrywanym ma być ożywiona poprzez
 * wywoływanie analyzeCell() z poprawnymi argumentami. 
*/

void analyzeEdgeCase1(list *above, list *below, list *current, rootList *pointerAbove,
                      rootList *pointerBelow, int column, int rowNumber, rootList **beginning, list **lastCurrentCell,
                      rootList **rootIndex, bool *edgeCase) {

    if (above->next != NULL && below->next != NULL)
        analyzeCell(above->next, below->next, current, pointerAbove, pointerBelow, column + 1, 0,
                    rowNumber, beginning, lastCurrentCell, rootIndex);
    else if (above->next != NULL)
        analyzeCell(above->next, below, current, pointerAbove, pointerBelow, column + 1, 0,
                    rowNumber, beginning, lastCurrentCell, rootIndex);
    else if (below->next != NULL)
        analyzeCell(above, below->next, current, pointerAbove, pointerBelow, column + 1, 0,
                    rowNumber, beginning, lastCurrentCell, rootIndex);
    else
        analyzeCell(above, below, current, pointerAbove, pointerBelow, column + 1, 0,
                    rowNumber, beginning, lastCurrentCell, rootIndex);

    (*edgeCase) = 1;
}

/**
 * Funkcja sprawdzająca czy komórki po ostatniej żywej komórce w wierszu rozpatrywanym mają być ożywione. Jest ona wywoływana
 * wtedy gdy przynajmniej jeden z pointerAbove i pointerBelow nie jest atrapą i wartość w above->column lub below->column jest większa 
 * lub równa wartości current->column kiedy current->next == NULL. 
*/

void analyzeEdgeCase2(list *above, list *below, list *current, rootList *pointerAbove,
                      rootList *pointerBelow, int column, int rowNumber, rootList **beginning, list **lastCurrentCell,
                      rootList **rootIndex, bool *edgeCase) {

    if (*edgeCase == 1)
        column += 1;

    if ((pointerAbove->state != 2 && pointerBelow->state != 2) && (column <= below->column || column <= above->column)) {
        int end = MAX(above->column, below->column);
        while (column < end + 1) {
            analyzeCell(above, below, current, pointerAbove, pointerBelow, column, 0,
                        rowNumber, beginning, lastCurrentCell, rootIndex);
            column += 1;
        }
    }
}

/**
 * Zaczynając od wartości column - 1, przy czym "column" to najmniejsza wartośc "column" pierwszego node'a z listy 
 * wiersza który rozpatrujemy (czyli pointerCurrent->right->column), wiersza powyżej (czyli pointerAbove->right->column) 
 * oraz wiersza poniżej (czyli pointerBelow->right->column), rozpatrujemy każdą możliwą wartość kolumny w której komórka 
 * może być ożywiona, czyli do największej wartości "column" ostatnich node'ów w wierszach które nie są atrapami. 
 * 
 * rootIndex wskazuje zawsze do tego wiersza do którego można dodać nowe komórki czyli pointerCurrent bądź nowy wiersz. 
 * 
 * edgeCase przechowuje informację czy pusta komórka po ostatniej żywej komórce w wierszu rozpatrywanym (czyli tym środkowym, czyli 
 * pointerCurrent) została już sprawdzona, aby potem nei sprawdzać jej drugi raz przy rozpatrywaniu dalszych komórek. 
 * 
 * above, below orz current wskazują zawsze do node'a który ma najmniejszą większą wartość w "column" od wartości zmiennej column w
 * swoich odpowiednich wierszach. 
*/

void analyzeRow(rootList *pointerAbove, rootList *pointerBelow, rootList *pointerCurrent,
                int rowNumber, rootList **beginning) {
    list *above = pointerAbove->right;
    list *below = pointerBelow->right;
    list *current = pointerCurrent->right;
    int column = MIN(MIN(above->column, below->column), current->column);
    bool edgeCase = 0;
    list *lastCurrentCell = current;
    rootList *rootIndex = pointerCurrent;

    analyzeCell(above, below, current, pointerAbove, pointerBelow, column - 1, 0,
                rowNumber, beginning, &lastCurrentCell, &rootIndex);

    while (above->next != NULL || current != NULL || below->next != NULL) {
        if (edgeCase == 0)
            analyzeNormalCase(above, below, current, pointerAbove, pointerBelow, column,
                              rowNumber, beginning, &lastCurrentCell, &rootIndex);
        edgeCase = 0;

        if (current != NULL && current->next == NULL && column == current->column)
            analyzeEdgeCase1(above, below, current, pointerAbove, pointerBelow, column,
                             rowNumber, beginning, &lastCurrentCell, &rootIndex, &edgeCase);
        movePointers(&above, &below, &current, &column);
    }
    analyzeEdgeCase2(above, below, current, pointerAbove, pointerBelow, column,
                     rowNumber, beginning, &lastCurrentCell, &rootIndex, &edgeCase);
}

/**
 * Wywołuje funkcję sprawdzającą czy komórki w "pustym" wierszu się ożywią z różnymi argumentami z zależności 
 * od tego czy sąsiadujące node'y mają kolejną lub poprzedzającą wartość "row". Jeśli różnica pomiędzy wartościami "row"
 * kolejnych node'ów jest większa bądź równa 2 to funkcja jest wywoływana dwa razy.
*/

void checkEmptyRow(rootList *pointer, rootList **beginning) {
    rootList *empty = createEmptyNode();

    if (pointer->prev == NULL) {
        analyzeRow(empty, pointer, empty, pointer->row - 1, beginning);
    } else if (pointer->next == NULL) {
        analyzeRow(pointer, empty, empty, pointer->row + 1, beginning);
    } else if (pointer->prev->row + 2 < pointer->row) {
        analyzeRow(pointer->prev, empty, empty, pointer->prev->row + 1, beginning);
        analyzeRow(empty, pointer, empty, pointer->row - 1, beginning);
    } else
        analyzeRow(pointer->prev, pointer, empty, pointer->row - 1, beginning);

    free(empty->right);
    free(empty);
}

/**
 * Wywołuje funkcję sprawdzającą czy stan komórek w "pełnym" wierszu się zmieni z różnymi argumentami w 
 * zależności od tego czy node poprzedzający ma wartość "row" równą pointer->row - 1 a node następny ma 
 * wartość "row" równą pointer->row + 1. Jeśli nie ma takiego/takich wierszy to funkcja jest wywoływana z 
 * atrapą "empty". 
*/

void checkFullRow(rootList *pointer, rootList **beginning) {
    rootList *empty = createEmptyNode();
    rootList *above;
    rootList *below;

    if (pointer->prev == NULL || pointer->prev->row + 1 < pointer->row || pointer->prev->state == 0)
        above = empty;
    else
        above = pointer->prev;

    if (pointer->next == NULL || pointer->next->row - 1 > pointer->row)
        below = empty;
    else
        below = pointer->next;

    analyzeRow(above, below, pointer, pointer->row, beginning);
    free(empty->right);
    free(empty);
}

/**
 * Usuwa element/node i odpowiednio zmienia wskaźniki sąsiadujących node'ów. 
*/

void deleteElement(list **branchPointer, rootList **pointer) {
    if ((*branchPointer)->prev != NULL && (*branchPointer)->next != NULL) {
        (*branchPointer)->prev->next = (*branchPointer)->next;
        (*branchPointer)->next->prev = (*branchPointer)->prev;
    } else if ((*branchPointer)->next != NULL) {
        (*pointer)->right = (*branchPointer)->next;
        (*branchPointer)->next->prev = NULL;
    } else if ((*branchPointer)->prev != NULL)
        (*branchPointer)->prev->next = NULL;
    else
        (*pointer)->right = NULL;
    free(*branchPointer);
}

/**
 * Usuwa cały wiersz jeśli w wyniku usuwania elementów stał się pusty. 
*/

void deleteRow(rootList **pointer, rootList **beginning) {
    if ((*pointer)->prev != NULL && (*pointer)->next != NULL) {
        (*pointer)->prev->next = (*pointer)->next;
        (*pointer)->next->prev = (*pointer)->prev;
    } else if ((*pointer)->prev != NULL) {
        (*pointer)->prev->next = NULL;
    } else if ((*pointer)->next != NULL) {
        (*pointer)->next->prev = NULL;
        (*beginning) = (*pointer)->next;
    } else {
        (*pointer)->next = NULL;
        (*pointer)->prev = NULL;
        *beginning = NULL;
    }
    free(*pointer);
}

/**
 * Przechodzi przez całą strukturę danych. Jeśli node ma stan (state) równy -1 to jest usuwany, jak ma stan równy 0
 * to zmieniany jest na stan równy 1. Jeśli w wyniku usunięć wiersz stał się pusty to jest on usuwany.
*/

void changeCellState(rootList **beginning) {
    rootList *pointer = *beginning;

    while (pointer != NULL) {
        if (pointer->state == 0)
            pointer->state = 1;

        list *branchPointer = pointer->right;
        rootList *pointerNext = NULL;

        if (pointer->next != NULL)
            pointerNext = pointer->next;

        while (branchPointer != NULL) {
            list *branchNext = NULL;
            if (branchPointer->next != NULL)
                branchNext = branchPointer->next;

            if (branchPointer->state == 0)
                branchPointer->state = 1;
            else if (branchPointer->state == -1)
                deleteElement(&branchPointer, &pointer);
            branchPointer = branchNext;
        }

        if (pointer->right == NULL)
            deleteRow(&pointer, beginning);
        pointer = pointerNext;
    }
}

/**
 * Funkcja tworzy *empty, czyli pointer na atrapę która reprezentuje "pusty" wiersz.  
 * Sprawdza czy różnica pomiędzy wartością "row" ostatniego i przedostatniego wiersza jest większa bądź równa 
 * 2, wtedy sprawdza czy komórki w pointer->prev->row + 1 i pointer->row - 1 wierszach się ożywią. Jeśli różnica 
 * jest równa 1 to sprawdza czy w tym pustym wierszu o wartości pointer->row - 1 ożywią się komórki. 
 * Sprawdza czy w ostatnim wierszu zmieni się stan komórek oraz sprawdza czy w wierszu o indeksie  pointer->row + 1
 * komórki się ożywią.  
*/

void checkLastRow(rootList **beginning, rootList *pointer) {
    rootList *empty = createEmptyNode();

    if (pointer->prev != NULL && (pointer->prev->row + 2 < pointer->row)) {
        analyzeRow(pointer->prev, empty, empty, pointer->prev->row + 1, beginning);
        analyzeRow(empty, pointer, empty, pointer->row - 1, beginning);
    } else if (pointer->prev != NULL && (pointer->prev->row + 1 < pointer->row))
        analyzeRow(pointer->prev, pointer, empty, pointer->row - 1, beginning);

    checkFullRow(pointer, beginning);
    checkEmptyRow(pointer, beginning);
    free(empty->right);
    free(empty);
}

/**
 * W pętli wykonuje proces obliczania nowej generacji numberOfGenerations razy. 
 * Najpierw sprawdza czy komórki w wierszu "nad" pierwszym będą ożywione. 
 * Zaczynając od pierwszego wiersza, dla każdego node'a w rootList'cie który reprezentuje jeden "pełny"
 * wiersz, tzn. wiersz w którym jest przynajmniej jedna żywa komórka, sprawdza które żywe komórki w tym 
 * wierszu przeżyją do następnej generacji, które umrą i które martwe zostaną ożywione. 
 * Jeżeli pomiędzy wartością "row" kolejnych node'ów jest różnica conajmniej równa 1, sprawdzany/ne jest/są
 * te "puste" wiersze pomięzy dwoma "pełnymi" wierszami. 
 * Na koniec stany komórek oraz usuwanie ich jest wywoływane poprzez changeCellState(). 
*/

void calculateXGenerations(int numberOfGenerations, rootList **beginning) {
    for (int i = 0; i < numberOfGenerations; i++) {
        rootList *pointer = *beginning;
        if (pointer != NULL) {
            checkEmptyRow(pointer, beginning);

            while (pointer->next != NULL) {
                if (pointer->prev != NULL && (pointer->prev->row + 1 < pointer->row))
                    checkEmptyRow(pointer, beginning);
                checkFullRow(pointer, beginning);
                pointer = pointer->next;
            }
            checkLastRow(beginning, pointer);
            changeCellState(beginning);
        }
    }
}

/**
 * Funkcja zwalniająca pamięć która jest zajmowana przez strukturę danych.
*/

void clearStructure(rootList *beginning) {
    rootList *pointer = beginning;
    while (pointer != NULL) {
        list *branchPointer = pointer->right;

        while (branchPointer != NULL) {
            list *branchNext = branchPointer->next;
            free(branchPointer);
            branchPointer = branchNext;
        }

        rootList *pointerNext = pointer->next;
        free(pointer);
        pointer = pointerNext;
    }
}

/**
 * Funkcja zmieniająca okno. 
*/

void changeWindow(char *input, int window[2], int *number) {
    window[0] = *number;
    scanf("%d", &(*number));
    window[1] = *number;
    *input = (char)getchar();
}

/**
 * Wczytywanie instrukcji z wejścia dopóki nie zostanie wczytany znak "." i wywołująca odpowiednie funkcje:
 * calculateXGenerations(), changeWindow() i printCoordinates(), i pokazuje aktualny stan okna funkcją
 * printCurrentState().
 * Na końcu wywołuje funkcję zwalniającą strukturę używając clearStructure().
*/

void readInstructions(rootList *beginning) {
    char input = (char)getchar();
    int window[2] = {1, 1};

    bool instructionsContinue = true;

    while (instructionsContinue) {
        printCurrentState(window, beginning);

        if (input == '.') {
            instructionsContinue = false;
        } else if (input == '\n') {
            calculateXGenerations(1, &beginning);
        } else {
            ungetc((int)input, stdin);
            int number;
            scanf("%d", &number);
            input = (char)getchar();

            if (input == ' ')
                changeWindow(&input, window, &number);
            else if (number == 0)
                printCoordinates(beginning);
            else
                calculateXGenerations(number, &beginning);
        }
        input = (char)getchar();
    }
    clearStructure(beginning);
}

/**
 * Tworzy pierwszy element rootList'y.
 * Wywołuje readingIntoRootList() aby wczytać początkowy stan generacji.
 * Wywołuje readInstructions() aby wczytać i wykonać instrukcje gracza. 
*/

int main(void) {
    rootList *head = malloc(sizeof(rootList));
    rootList *beginning = head;
    head->prev = NULL;
    head->next = NULL;
    readingIntoRootList(head);
    readInstructions(beginning);
    return 0;
}
