/*
 * =====================================================
 * Piąty projekt zaliczeniowy z
 * przedmiotu Kurs Programowania w C++ (1000-213bCPP).
 *
 * Plik nagłówkowy modułu 'stack'.
 *
 * Plik zawiera implementacje szablonu kontenera
 * zachowującego się jak stos, w którym każdy
 * element składa się z klucza i wartości.
 *
 * Autorzy:
 * Natalia Junkiert <nj448267@students.mimuw.edu.pl>
 * Dawid Pawlik     <dp450244@students.mimuw.edu.pl>
 * =====================================================
 */


/* Początek deklaracji elementów modułu stack.h */

#ifndef STACK_H
#define STACK_H


/* Pliki nagłówkowe używane przy implementacji modułu */

#include <map>
#include <list>
#include <memory>
#include <stdexcept>
#include <utility>


/* Przestrzeń nazw implementowanej klasy 'stack' */

namespace cxx {

    /* Deklaracje użycia wybranych elementów z przestrzeni std */

    using std::map;
    using std::list;
    using std::pair;
    using std::invalid_argument;
    using std::forward_iterator_tag;
    using std::move;
    using std::shared_ptr;
    using std::make_shared;


    /*
     * Klasa 'stack' implementuje kontener przechowujący klucze i wartości,
     * pamiętający kolejność dodawania wg. zasady LIFO, spełniający założenia
     * copy-on-wright oraz korzystający z odporności na wyjątki dzięki RAII.
     */
    template <typename K, typename V>
    class stack {
        public:

            /*
             * Konstruktor domyślny klasy 'stack'.
             *
             * Może rzucać wyjątek 'bad_alloc'.
             */
            stack() : ptr{make_shared<data>()} {}

            /*
             * Konstruktor kopiujący klasy 'stack'.
             *
             * Może rzucać wyjątek 'bad_alloc'.
             */
            stack(stack const& other) {
                if (other.is_shareable) {
                    ptr = other.ptr;
                } else {
                    ptr = make_shared<data>(*other.ptr.get());
                }
            }

            /*
             * Konstruktor przenoszący klasy 'stack'.
             */
            stack(stack&& other) noexcept : ptr(move(other.ptr)) {}

            /*
             * Destruktor klasy 'stack' pozostaje domyślny.
             */
            ~stack() noexcept = default;

            /*
             * Operator kopiujący przypisania klasy 'stack'.
             *
             * Może rzucać wyjątek 'bad_alloc'.
             */
            stack& operator=(stack other) {
                if (this == &other) {
                    return *this;
                }

                if (other.is_shareable) {
                    ptr = other.ptr;
                } else {
                    ptr = make_shared<data>(*other.ptr.get());
                }

                return *this;
            }

            /*
             * Funkcja wywołuje funkcję 'push' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack' wykorzystując
             * mechanizm RAII.
             *
             * Może rzucać wyjątek 'invalid_argument' oraz 'bad_alloc'.
             */
            void push(K const& key, V const& value) {
                copy_ptr(&ptr, is_shareable);

                is_shareable = true;

                exception_guard push(ptr, key, value);
                push.drop_rollback();
            }

            /*
             * Funkcja wywołuje funkcję 'pop' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack' wykorzystując
             * mechanizm RAII.
             *
             * Może rzucać wyjątek 'invalid_argument' oraz 'bad_alloc'.
             */
            void pop() {
                copy_ptr(&ptr, is_shareable);

                is_shareable = true;

                exception_guard pop(ptr);
                pop.drop_rollback();
            }

            /*
             * Funkcja wywołuje funkcję 'pop' z kluczem na przechowywanym
             * wskaźniku 'ptr' do struktury danych implementującej 'stack'
             * wykorzystując mechanizm RAII.
             *
             * Może rzucać wyjątek 'invalid_argument' oraz 'bad_alloc'.
             */
            void pop(K const& key) {
                copy_ptr(&ptr, is_shareable);

                is_shareable = true;

                exception_guard pop(ptr, key);
                pop.drop_rollback();
            }

            /*
             * Funkcja wywołuje funkcję 'front' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             *
             * Może rzucać wyjątek 'invalid_argument' oraz 'bad_alloc'.
             */
            pair<K const&, V&> front() {
                copy_ptr(&ptr, is_shareable);

                is_shareable = false;

                return ptr->front();
            }

            /*
             * Funkcja wywołuje funkcję 'front' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             *
             * Może rzucać wyjątek 'invalid_argument'.
             */
            pair<K const&, V const&> front() const {
                return ptr->front();
            }

            /*
             * Funkcja wywołuje funkcję 'front' z kluczem na przechowywanym
             * wskaźniku 'ptr' do struktury danych implementującej 'stack'.
             *
             * Może rzucać wyjątek 'invalid_argument' oraz 'bad_alloc'.
             */
            V& front(K const& key) {
                copy_ptr(&ptr, is_shareable);

                is_shareable = false;

                return ptr->front(key);
            }

            /*
             * Funkcja wywołuje funkcję 'front' z kluczem na przechowywanym
             * wskaźniku 'ptr' do struktury danych implementującej 'stack'.
             *
             * Może rzucać wyjątek 'invalid_argument'.
             */
            V const& front(K const& key) const {
                return ptr->front(key);
            }

            /*
             * Funkcja wywołuje funkcję 'size' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             */
            size_t size() const noexcept {
                return ptr->size();
            }

            /*
             * Funkcja wywołuje funkcję 'count' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             *
             * Może rzucać wyjątek 'invalid_argument'.
             */
            size_t count(K const& key) const {
                return ptr->count(key);
            }

            /*
             * Funkcja wywołuje funkcję 'clear' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             *
             * Może rzucać wyjątek 'bad_alloc'.
             */
            void clear() {
                copy_ptr(&ptr, is_shareable);

                is_shareable = true;

                ptr->clear();
            }


            /*
             * Klasa implementująca iterator analogiczny do const_iterator
             * z biblioteki standardowej.
             *
             * Implementowany iterator przechodzi po wartościach kluczy w
             * implementowanym kontenerze 'stack'.
             */
            class const_iterator {
                public:

                    /* Typy potrzebne do spełnienia konceptu forward_iterator */

                    using iterator_category = forward_iterator_tag;
                    using value_type = const K;
                    using map_t = map<value_type, list<V>>;
                    using map_iterator = typename map_t::const_iterator;
                    using difference_type = map_t::difference_type;
                    using pointer = value_type*;
                    using reference = value_type&;


                    /*
                     * Deklaracja użycia domyślnych konstruktorów i
                     * operatorów przypisania w celu spełnienie
                     * konceptu forward_iterator.
                     */

                    const_iterator() = default;
                    const_iterator(const const_iterator& other) = default;
                    const_iterator(const_iterator&& other) noexcept = default;

                    const_iterator& operator=
                                    (const const_iterator& other) = default;
                    const_iterator& operator=
                                    (const_iterator&& other) noexcept = default;


                    /*
                     * Konstruktor kopiujący klasy 'const_iterator'.
                     */
                    const_iterator(const map_iterator& it) : iter(it) {}

                    /*
                     * Konstruktor przenoszący klasy 'const_iterator'.
                     */
                    const_iterator(map_iterator&& it) noexcept :
                                                      iter(move(it)) {}

                    /*
                     * Operator przekazujący referencje na wskazywaną wartość.
                     */
                    reference operator*() const noexcept {
                        return iter->first;
                    }

                    /*
                     * Operator przekazujący adres wskazywanej wartości.
                     */
                    pointer operator->() const noexcept {
                        return &(iter->first);
                    }

                    /*
                     * Operator preinkrementacji iteratora.
                     */
                    const_iterator& operator++() noexcept {
                        ++iter;
                        return *this;
                    }

                    /*
                     * Operator postinkrementacji iteratora.
                     */
                    const_iterator operator++(int) noexcept {
                        const_iterator result(*this);
                        operator++();
                        return result;
                    }

                    /*
                     * Operator porównania iteratorów.
                     */
                    friend bool operator==(const_iterator const& a,
                                           const_iterator const& b) noexcept {
                        return a.iter == b.iter;
                    }

                    /*
                     * Operator nierówności iteratorów.
                     */
                    friend bool operator!=(const_iterator const& a,
                                           const_iterator const& b) noexcept {
                        return !(a == b);
                    }

                private:
                    map_iterator iter;
            };

            /*
             * Funkcja wywołuje funkcję 'cbegin' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             */
            const_iterator cbegin() const noexcept {
                return ptr->cbegin();
            }

            /*
             * Funkcja wywołuje funkcję 'cend' na przechowywanym wskaźniku
             * 'ptr' do struktury danych implementującej 'stack'.
             */
            const_iterator cend() const noexcept {
                return ptr->cend();
            }


            /*
             * Klasa 'data' implementuje mechanizmy przechowywania
             * i modyfikacji danych klasy 'stack'.
             */
            class data {
                public:

                    /*
                     * Konstruktor domyślny klasy 'data'.
                     */
                    data() : ordered_keys(), fifo_list(), iterators_in_list() {}

                    /*
                     * Konstruktor kopiujący klasy 'data'.
                     */
                    data(const data& other) : ordered_keys{}, fifo_list{},
                                              iterators_in_list{} {
                        for (auto it : other.fifo_list) {
                            K key = it.first->first;
                            V value = *(it.second);

                            if (!ordered_keys.contains(key)) {
                                ordered_keys.insert({key, list<V>()});
                            }
                            ordered_keys.at(key).push_back(value);

                            auto key_it = ordered_keys.find(key);
                            fifo_list.push_back({key_it,
                                                 --(key_it->second.end())});

                            iterators_in_list[key_it]
                                             .push_back((--fifo_list.end()));
                        }
                    }

                    /*
                    * Funkcja dodaje do kontenera 'stack'
                    * parę wartości {'key', 'value'}.
                    */
                    void push(K const& key, V const& value) {
                        if (!ordered_keys.contains(key)) {
                            ordered_keys.insert({key, list<V>()});
                        }

                        V const& value2 = value;

                        ordered_keys.at(key).insert(ordered_keys.at(key).begin(),
                                                 value2);

                        fifo_list.push_front({ordered_keys.find(key),
                                              ordered_keys.at(key).begin()});

                        iterators_in_list[ordered_keys.find(key)]
                                         .push_front(fifo_list.begin());
                    }

                    /*
                    * Funkcja usuwa ostatnio dodaną
                    * do kontenera 'stack' wartość.
                    */
                    void pop() {
                        if (fifo_list.empty()) {
                            throw invalid_argument("Error: Stack is empty.");
                        }

                        auto first = fifo_list.front();

                        iterators_in_list[first.first].pop_front();

                        if (iterators_in_list[first.first].empty()) {
                            iterators_in_list.erase(first.first);
                        }

                        ordered_keys[first.first->first].erase(first.second);

                        if (ordered_keys[first.first->first].empty()) {
                            ordered_keys.erase(first.first->first);
                        }

                        fifo_list.pop_front();
                    }

                    /*
                    * Funkcja usuwa ostatnio dodaną do kontenera
                    * 'stack' wartość spod klucza 'key'.
                    */
                    void pop(K const& key) {
                        if (fifo_list.empty()) {
                            throw invalid_argument("Error: Stack is empty.");
                        }

                        if (!ordered_keys.contains(key)) {
                            throw invalid_argument("Error: Invalid argument.");
                        }

                        auto key_it = ordered_keys.find(key);

                        fifo_list.erase(iterators_in_list[key_it].front());

                        iterators_in_list[key_it].pop_front();

                        if (iterators_in_list[key_it].empty()) {
                            iterators_in_list.erase(key_it);
                        }

                        ordered_keys.at(key).erase(key_it->second.begin());

                        if (ordered_keys.at(key).empty()) {
                            ordered_keys.erase(key);
                        }
                    }

                    /*
                    * Funkcja przekazuje parę ostatnio dodanej
                    * do kontenera 'stack' wartości oraz jej klucza.
                    */
                    pair<K const&, V&> front() {
                        if (fifo_list.empty()) {
                            throw invalid_argument("Error: Stack is empty.");
                        }

                        const K& key = fifo_list.front().first->first;
                        V& value = *(fifo_list.front().second);
                        pair<K const&, V&> result{key, value};

                        return result;
                    }

                    /*
                    * Funkcja przekazuje parę ostatnio dodanej
                    * do kontenera 'stack' wartości oraz jej klucza.
                    */
                    pair<K const&, V const&> front() const {
                        if (fifo_list.empty()) {
                            throw invalid_argument("Error: Stack is empty.");
                        }

                        const K& key = fifo_list.front().first->first;
                        const V& value = *(fifo_list.front().second);
                        pair<K const&, V const&> result{key, value};

                        return result;
                    }

                    /*
                    * Funkcja przekazuje ostatnio dodaną wartość
                    * do kontenerza 'stack' o kluczu 'key'.
                    */
                    V& front(K const& key) {
                        if (!ordered_keys.contains(key)) {
                            throw invalid_argument("Error: Invalid argument.");
                        }

                        return (V&)ordered_keys.at(key).front();
                    }

                    /*
                    * Funkcja przekazuje ostatnio dodaną wartość
                    * do kontenerza 'stack' o kluczu 'key'.
                    */
                    V const& front(K const& key) const {
                        if (!ordered_keys.contains(key)) {
                            throw invalid_argument("Error: Invalid argument.");
                        }

                        return ordered_keys.at(key).front();
                    }

                    /*
                    * Funkcja przekazuje liczbę wartości
                    * dodanych do kontenera 'stack'.
                    */
                    size_t size() const noexcept {
                        return fifo_list.size();
                    }

                    /*
                    * Funkcja przekazuje liczbę wartości o kluczu 'key'
                    * dodanych do kontenera 'stack'.
                    */
                    size_t count(K const& key) const {
                        if (!ordered_keys.contains(key)) {
                            return 0;
                        }

                        return ordered_keys.at(key).size();
                    }

                    /*
                    * Funkcja usuwa wszystkie wartości uprzednio
                    * przechowywane w kontenerze 'stack'.
                    */
                    void clear() {
                        ordered_keys.clear();
                        fifo_list.clear();
                        iterators_in_list.clear();
                    }

                    /*
                     * Funkcja przekazuje iterator wskazujący na pierwszy
                     * klucz przechowywany w kontenerze 'stack'.
                     */
                    const_iterator cbegin() const noexcept {
                        return const_iterator(ordered_keys.cbegin());
                    }

                    /*
                     * Funkcja przekazuje iterator wskazujący za ostatni
                     * klucz przechowywany w kontenerze 'stack'.
                     */
                    const_iterator cend() const noexcept {
                        return const_iterator(ordered_keys.cend());
                    }

                private:

                    /* Deklaracje użycia aliasów typów */

                    using ordered_map = map<K, list<V>>;
                    using by_value = typename list<V>::iterator;
                    using by_key = typename ordered_map::iterator;
                    using list_iterator =
                          typename list<pair<by_key, by_value>>::iterator;

                    /*
                    * Struktura porównująca iteratory.
                    */
                    struct iterator_compare {
                        bool operator()(const by_key a, const by_key b) const {
                            return a->first < b->first;
                        }
                    };

                    struct key_compare {
                        bool operator()(const K a, const K b) const {
                            return a < b;
                        }
                    };

                    /* Kontenery używane do implementacji */

                    map<K, list<V>, key_compare> ordered_keys;
                    list<pair<by_key, by_value>> fifo_list;
                    map<by_key, list<list_iterator>,
                        iterator_compare> iterators_in_list;

            };

        private:

            /*
             * Wskaźnik do danych przechowywanych w kontenerze 'stack'
             */
            shared_ptr<data> ptr;

            bool is_shareable = true;


            /*
             * Klasa implementująca model RAII
             */
            class exception_guard {
                public:

                    /*
                     * Konstruktor okalający wywołanie funkcji 'push'.
                     */
                    explicit exception_guard(shared_ptr<data> ptr,
                                             K const& key, V const& value) :
                                             data_ptr(&ptr), saved_ptr(ptr),
                                             rollback(false) {
                        (*data_ptr)->push(key, value);
                        rollback = true;
                    }

                    /*
                     * Konstruktor okalający wywołanie funkcji 'pop'.
                     */
                    explicit exception_guard(shared_ptr<data> ptr) :
                                             data_ptr(&ptr), saved_ptr(ptr),
                                             rollback(false) {
                        (*data_ptr)->pop();
                        rollback = true;
                    }

                    /*
                     * Konstruktor okalający wywołanie funkcji 'pop' z kluczem.
                     */
                    explicit exception_guard(shared_ptr<data> ptr,
                                             K const& key) : data_ptr(&ptr),
                                             saved_ptr(ptr), rollback(false) {
                        (*data_ptr)->pop(key);
                        rollback = true;
                    }

                    /* Deklarujemy brak możliwości kopiowania strażników */

                    exception_guard(exception_guard const&) = delete;
                    exception_guard& operator=(exception_guard const&) = delete;


                    /*
                     * Destruktor przywracający wskaźnik na dane przechowywane
                     * w kontenerze 'stack' do stanu pierwotnego.
                     */
                    ~exception_guard() noexcept {
                        if (rollback) {
                            *data_ptr = saved_ptr;
                        }
                    }

                    /*
                     * Funkcja odwołująca konieczność
                     * przywrócenia stanu pierwotnego.
                     */
                    void drop_rollback() noexcept {
                        rollback = false;
                    }

                private:

                    /*
                     * Wskaźnik na wskaźnik danych przechowywanych w 'stack'.
                     */
                    shared_ptr<data>* data_ptr;

                    /*
                     * Wskaźnik na pierwotne dane przed wykonaniem funkcji.
                     */
                    shared_ptr<data> saved_ptr;

                    /*
                     * Flaga wycofywania zmian.
                     */
                    bool rollback;

            };

            /*
             * Funkcja tworzy głęboką kopię elementów wskazywanych przez 'ptr'
             * jeśli istnieje inny wskaźnik współdzielący dane z 'ptr'.
             */
            void copy_ptr(shared_ptr<data>* ptr, bool is_shareable) {
                if ((*ptr).use_count() > 1 && is_shareable) {
                    *ptr = make_shared<data>(*(*ptr).get());
                }
            }
    };

} // Koniec przestrzeni nazw cxx


#endif // Koniec header-guard