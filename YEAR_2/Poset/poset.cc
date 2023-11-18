#include <iostream>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include "poset.h"
using std::string;
using std::cerr;
using std::endl;
using std::unordered_set;
using std::unordered_map;
using std::to_string;

#ifdef NDEBUG
  bool constexpr debug = false;
#else
  bool constexpr debug = true;
#endif

// posets to mapa map. Pierwsza mapa odwzorowuje identyfikator na mapę 
// elementów typu string. Druga mapa odwzorowuje string na set ciągów, 
// gdzie pierwsza zawiera wszystkie takie elementy a, że a < b.
using posets_map = unordered_map<uint32_t, 
                    unordered_map<string,
                    unordered_set<const string*>>>;

namespace {
    //unit32_t id_num = 0;
    uint32_t& id_numy() {
        static uint32_t id_n = 0;
        return id_n;
    }

    posets_map& posets() {
        static posets_map* posets = new posets_map();
        return *posets;
    }

    // Sprawdza, czy dana wartość znajduje się w posecie. Zwraca true, jeśli
    // to prawda, false w przeciwnym wypadku.
    bool is_in_poset(uint32_t id, string value) {
        return posets()[id].find(value) != posets()[id].end();
    }

    // Sprawdza czy poset istnieje. Jeżeli nie istnieje oraz kompilowany 
    // został plik z opcją DEBUG wypisuje odpowiedni komunikat.
    bool poset_exists(unsigned long id, string function_name) {
        bool found = posets().find(id) != posets().end();

        if constexpr (debug) {
            if (!found) {
                cerr << function_name +": poset " + to_string(id) +
                    " does not exist" << endl;
            }
        }

        return found;
    }

    // Zwraca wskaźnik na string/element w posecie.
    const string * get_poset_value(uint32_t id, char const * value) {
        return &posets()[id].find(value)->first;
    }

    // Funkcja wypisująca odpowiedni komunikat w zależności od ilości 
    // argumentów.
    void print_three_arguments(string function_name, uint32_t id,
                                char const *value1, char const *value2) {
        if (value1 == NULL && value2 == NULL) {
            if constexpr (debug) {
                cerr << function_name + "(" + to_string(id) 
                    + ", NULL, NULL)" << endl;
            }
            return;
        } 
        else if (value1 == NULL) {
            if constexpr (debug) {
                cerr << function_name + "(" + to_string(id) 
                    + ", NULL, \"" + value2 + "\")" << endl;
            }
            return;
        } 
        else if (value2 == NULL) {
            if constexpr (debug) {
                cerr << function_name + "(" + to_string(id) + ", \"" 
                    + value1 + "\", NULL)" << endl;
            }
            return;
        }

        if constexpr (debug) {
            cerr << function_name + "(" + to_string(id) + ", \"" 
                + value1 + "\"" + ", \"" + value2 + "\")" << endl;
        }
    }

    void print_two_arguments(string function_name, uint32_t id,
                            char const *value) {
        if (value == NULL) {
            if constexpr (debug) {
                cerr << function_name + "(" + to_string(id) 
                    + ", NULL)" << endl;
            }
            return;
        }

        if constexpr (debug) {
            cerr << function_name + "(" + to_string(id) + ", \"" 
                + value + "\")" << endl;
        }
    }

    void print_one_argument(string function_name, uint32_t id) {
        cerr << function_name + "(" + to_string(id) + ")" 
            << endl;
    }

    // Funkcje sprawdzające poprawność danych pasowanych do funkcji ich 
    // wywołujących. Wypisują stosowny komunikat jeżeli został plik 
    // skompilowany z opcją DEBUG.
    bool validate_three_arguments(string function_name, uint32_t id,
                                char const *value1, char const *value2) {
        bool return_value = true;

        if (!poset_exists(id, function_name)) {
            return_value = false;
        }

        if (!value1) {
            if constexpr (debug) {
                cerr << function_name +": invalid value1 (NULL)" 
                    << endl;
            }
            return_value = false;
        }

        if (!value2) {
            if constexpr (debug) {
                cerr << function_name +": invalid value2 (NULL)" 
                    << endl;
            }
            return_value = false;
        }

        if (!return_value) {
            return return_value;
        }

        if (!is_in_poset(id, value1)) {
            if constexpr (debug) {
                cerr << function_name +": poset " + to_string(id) 
                    + ", element \"" + value1 + "\" does not exist" 
                    << endl;
            }
            return false;
        }

        if (!is_in_poset(id, value2)) {
            if constexpr (debug) {
                cerr << function_name +": poset " + to_string(id) 
                    + ", element \"" + value2 + "\" does not exist" 
                    << endl;
            }
            return false;
        }

        return return_value;
    }

    bool validate_two_arguments(string function_name, uint32_t id, 
                                char const *value) {
        bool return_value = true;

        if (!poset_exists(id, function_name)) {
            return false;
        }

        if (value == nullptr) {
            if constexpr (debug) {
                cerr << function_name +": invalid value (NULL)" 
                    << endl;
            }
            return_value = false;
        } 
        else if (!is_in_poset(id, value)) {
            if constexpr (debug) {
                cerr << function_name +": poset " + to_string(id) 
                    + ", element \"" + value + "\" does not exist" << endl;
            }
            return_value = false;
        }

        return return_value;
    }

    // Funkcja sprawdza, czy istnieje relacja między value1 i value2 używając
    // algorytmu BFS. Sprawdza czy value2 jest w zbiorze wartości osiągalnych
    // z value1 idąc po relacjach bezpośrednich, w taki sposób sprawdzając 
    // relacje pośrednie i bezpośrednie. Zwraca true jeżeli tak jest, false
    // w przeciwnym wypadku.
    // Funkcja zakłada że value1 i value2 są w posecie i że poset istnieje.
    bool relation_exists(uint32_t id, char const *value1, 
                            char const *value2) {
        const string* value1_pointer = get_poset_value(id, value1);
        const string* value2_pointer = get_poset_value(id, value2);
        if(value1_pointer == value2_pointer) {
            return true;
        } 

        unordered_set<const string*> visited;
        unordered_set<const string*> to_visit;

        visited.insert(value1_pointer);

        for (const string* value : posets()[id].at(value1)) {
            to_visit.insert(value);
        }

        while (!to_visit.empty()) {
            const string* current = *to_visit.begin();
            to_visit.erase(to_visit.begin());
            visited.insert(current);

            if (current == value2_pointer) {
                return true;
            }

            for (const string *value : posets()[id].at(*current)) {
                if (visited.find(value) == visited.end()) {
                    to_visit.insert(value);
                }
            }
        }

        return false;
    }

    // Funkcja dodaje odpowiednie relacje po usunęciu elementu parent_value.
    // Wszystkim ojcom parent_value przekazuje wszystkich synów parent_value
    // jako swoich synów.
    void reparent_children(uint32_t id, char const *parent_value) {
        string parent_string(parent_value);
        const string* parent_pointer = get_poset_value(id, parent_value);

        unordered_set<const string*> parent_elements 
            = posets()[id].at(parent_string);

        for (auto child = posets()[id].begin(); child != posets()[id].end(); 
                child++) {

            if (child->second.find(parent_pointer) 
                    != child->second.end()) {

                for (auto parent = parent_elements.begin(); 
                        parent != parent_elements.end(); ++parent) {
                    child->second.insert(*parent);
                }

                child->second.erase(parent_pointer);
            }
        }                          
    }

    // Sprawdza czy usunięcie relacji między value1 i value2 nie zaburzy
    // częściowego porządku reprezentowanego przez poseta. Zwraca true jeżeli
    // nie zaburza, false w przeciwnym wypadku.
    bool del_not_disturb(uint32_t id, char const *value1, 
                            char const *value2) {
        const string* value1_pointer = get_poset_value(id, value1);
        const string* value2_pointer = get_poset_value(id, value2);

        if(value1_pointer == value2_pointer) {
            return false;
        }

        for (const string* value : posets()[id].at(value1)) {
            // Usuwa relację, gdy value1 i value2 są w bezpośredniej relacji.
            if (value == value2_pointer) {
                posets()[id].at(value1).erase(value2_pointer);

                // Usuwa relację a < b.

                for(const string* value : posets()[id].at(value2)) {
                    // Przekazuje b wszystkich synów a. 
                    posets()[id].at(value1).insert(value);
                }

                for (auto child = posets()[id].begin(); 
                                    child != posets()[id].end(); ++child) {
                    if(child->second.find(value1_pointer) 
                                    != child->second.end()) {
                        // Wszystkim ojcom b przekazuje a.
                        child->second.insert(value2_pointer);   
                    }
                }

                return true;
            }
        }

        return false;
    }
}

// Tworzy nowy poset i zwraca jego ID.
unsigned long cxx::poset_new(void) {
    if constexpr (debug) {
        cerr << "poset_new()" << endl;
    }   

    posets().insert({id_numy(), unordered_map<string, 
                    unordered_set<const string*>>()});


    if constexpr (debug) {
        cerr << "poset_new: poset " << id_numy() << " created" 
        << endl;
    } 

    id_numy()++;
    return id_numy() - 1;
}

// Jeżeli istnieje poset o identyfikatorze id, usuwa go, a w przeciwnym
// przypadku nic nie robi.
void cxx::poset_delete(unsigned long id) {
    if constexpr (debug) {
        print_one_argument("poset_delete", id);
    }

    if (poset_exists(id, "poset_delete")) {
        posets().erase(id);
        
        if constexpr (debug) {
            cerr << "poset_delete: poset " << id << " deleted" 
            << endl;
        }
    }
}

// Jeżeli istnieje poset o identyfikatorze id, to wynikiem jest liczba jego
// elementów, a w przeciwnym przypadku 0.
size_t cxx::poset_size(unsigned long id) {
    if constexpr (debug) {
        print_one_argument("poset_size", id);
    } 

    if (poset_exists(id, "poset_size")) {
        if constexpr (debug) {
            cerr << "poset_size: poset " << id << " contains " 
            << posets()[id].size() << " element(s)" << endl;
        } 

        return posets()[id].size();
    }

    return 0;
}

// Jeżeli istnieje poset o identyfikatorze id i element value nie należy do
// tego zbioru, to dodaje element do zbioru, a w przeciwnym przypadku nic nie
// robi. Nowy element nie jest w relacji z żadnym innym elementem. Wynikiem
// jest true, gdy element został dodany, a false w przeciwnym przypadku.
bool cxx::poset_insert(unsigned long id, char const *value) {
    if constexpr (debug) {
        print_two_arguments("poset_insert", id, value);
    }

    if (value == NULL){
        if constexpr (debug) {
            cerr << "poset_insert: invalid value (NULL)" << endl;
        }
        return false;
    }

    const string value_string(value);

    if (poset_exists(id, "poset_insert")) {
        if (!is_in_poset(id, value)) {
            posets()[id][value_string] = 
                unordered_set<const string*>();

            if (is_in_poset(id, value)) {
                if constexpr (debug) {
                    cerr << "poset_insert: poset " << id << ", element \""
                    << value << "\" inserted"<< endl;
                }
                return true;
            }
        }
        else {
            if constexpr (debug) {
                cerr << "poset_insert: poset " << id << ", element \"" 
                << value << "\" already exists"<< endl;
            }
            return false;
        }
    }
    
    return false;
}

// Jeżeli istnieje poset o danym identyfikatorze id oraz value do niego należy
// to usuwa element ze zbioru ale pozostawia wszystkie relacje przechodnie 
// w których ten element brał udział. Wynikiem jest true, gdy element został
// usunięty, w przeciwnym wypadku false.
bool cxx::poset_remove(unsigned long id, char const *value) {
    if constexpr (debug) {
        print_two_arguments("poset_remove", id, value);
    }

    if (validate_two_arguments("poset_remove", id, value)) {
        reparent_children(id, value);

        posets()[id].erase(value);

        if constexpr (debug) {
            cerr << "poset_remove: poset " << id << ", element \"" << 
            value << "\" removed" << endl;
        }

        return true;
    }

    return false;
}

// Jeżeli istnieje poset o podanym identyfikatorze id, oraz elementy value1 i 
// value2 należą do niego oraz nie są w relacji ze sobą, to dodaje relację taką
// że value1 poprzedza value2. Wynikiem jest true, gdy relacja została dodana,
// a false w przeciwnym wypadku. Wynikiem jest false jeżeli elementy już były w
// relacji.
bool cxx::poset_add(unsigned long id, char const *value1, char const *value2) {
    if constexpr (debug) {
        print_three_arguments("poset_add", id, value1, value2);
    }

    if (validate_three_arguments("poset_add", id, value1, value2)) {
        if (relation_exists(id, value1, value2) || 
            relation_exists(id, value2, value1)) {

            if constexpr (debug) {
                cerr << "poset_add: poset " << id << ", relation (\"" << 
                value1 << "\", \"" << value2 << "\") cannot be added" 
                << endl;
            }
            return false;
        }

        posets()[id].at(value1).insert(get_poset_value(id, value2));

        if constexpr (debug) {
            cerr << "poset_add: poset " << id << ", relation (\"" << 
            value1 << "\", \"" << value2 << "\") added" << endl;
        }

        return true;
    } 

    return false;
}

// Jeżeli istnieje poset o identyfikatorze id, elementy value1 i value2
// należą do tego zbioru, element value1 poprzedza element value2 oraz
// usunięcie relacji między elementami value1 i value2 nie zaburzy warunków
// bycia częściowym porządkiem, to usuwa relację między tymi elementami,
// a w przeciwnym przypadku nic nie robi. Wynikiem jest true, gdy relacja
// została zmieniona, a false w przeciwnym przypadku.
bool cxx::poset_del(unsigned long id, char const *value1, char const *value2) {
    if constexpr (debug) {
        print_three_arguments("poset_del", id, value1, value2);
    }

    if (validate_three_arguments("poset_del", id, value1, value2)) {
        if (relation_exists(id, value1, value2) 
            && del_not_disturb(id, value1, value2)) {
            if constexpr (debug) {
                cerr << "poset_del: poset " << id << ", relation (\"" << 
                value1 << "\", \"" << value2 << "\") deleted" 
                << endl;
                }

            if (relation_exists(id, value1, value2)) {
                return false;
            }

            return true; 
        }
        else { 
            if constexpr (debug) {
                cerr << "poset_del: poset " << id << ", relation (\"" << 
                value1 << "\", \"" << value2 << "\") cannot be deleted" 
                << endl;
            }
        }
    } 
    
    return false;
}

// Funkcja zwraca true jeżeli poset o identyfikatorze id istnieje, value1 i 
// value2 istnieją w nim oraz value 1 poprzedza value2. W przeciwnym wypadku
// zwraca false.
bool cxx::poset_test(unsigned long id, char const *value1, char const *value2) {
    if constexpr (debug) {
        print_three_arguments("poset_test", id, value1, value2);
    }

    if (validate_three_arguments("poset_test", id, value1, value2)) {
        if (relation_exists(id, value1, value2)) {
            if constexpr (debug) {
                cerr << "poset_test: poset " << id << ", relation (\"" << 
                value1 << "\", \"" << value2 << "\") exists" << endl;
            }

            return true; 
        } 
        else {
            if constexpr (debug) {
                cerr << "poset_test: poset " << id << ", relation (\"" << 
                value1 << "\", \"" << value2 << "\") does not exist" 
                << endl;
            }
        }
    }

    return false;
}

// Jeżeli istnieje poset o identyfikatorze id, usuwa wszystkie jego elementy
// oraz relacje między nimi, a w przeciwnym przypadku nic nie robi.
void cxx::poset_clear(unsigned long id) {
    if constexpr (debug) {
        print_one_argument("poset_clear", id);
    }

    if (poset_exists(id, "poset_clear")) {
        posets()[id].clear();
        if constexpr (debug) {
            cerr << "poset_clear: poset " << id << " cleared" << endl;
        }
    }
}
