/* Początek deklaracji elementów modułu college.h */

#ifndef COLLEGE_H
#define COLLEGE_H

/* Pliki nagłówkowe używane przy implementacji modułu */

#include <algorithm>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

/*
    Klasa 'Course' reprezentująca kurs na uczelni. Udostępnia metody is_active()
    oraz change_activeness(bool), które pozwalają na sprawdzenie czy kurs jest
    aktywny oraz zmianę jego stanu aktywności. Udostępnia również metodę
    get_name(), która zwraca nazwę kursu.
*/

class Course {
private:
    std::string name;
    bool active;

public:
    Course(const std::string& name, bool active = true)
        : name(name), active(active) {}

    void change_activeness(bool new_activeness) { active = new_activeness; }

    std::string get_name() const { return name; }

    bool is_active() const { return active; }
};

/*
    Klasa 'Person' reprezentująca osobę na uczelni. Udostępnia metody get_name()
    oraz get_surname(), które zwracają odpowiednio imię oraz nazwisko osoby.
*/

class Person {
private:
    std::string name;
    std::string surname;

protected:
    struct pcourse_cmp {
        bool operator()(std::shared_ptr<Course> p1,
                        std::shared_ptr<Course> p2) const {
            return p1->get_name() < p2->get_name();
        }
    };

public:
    Person(const std::string& n, const std::string& s) : name(n), surname(s){};
    Person() = delete;
    virtual ~Person() = default;

    std::string get_name() const { return name; };

    std::string get_surname() const { return surname; };
};

/*
    Klasa 'Student' reprezentująca studenta na uczelni. Wirtualnie dziedziczy po
    klasie 'Person'.
*/

class Student : virtual public Person {
private:
    bool active;
    std::set<std::shared_ptr<Course>, pcourse_cmp> courses;

public:
    Student(const std::string& name, const std::string& surname,
            bool active = true)
        : Person(name, surname), active(active) {}
    Student() = delete;
    virtual ~Student() = default;

    /*
        Zmienia stan aktywności studenta na new_activeness.
    */
    void change_activeness(bool new_activeness) { active = new_activeness; }

    /*
        Zwraca true jeśli student jest aktywny, false w przeciwnym wypadku.
    */
    bool is_active() const { return active; };

    /*
        Zwraca referencję do zbioru kursów, na które student jest zapisany.
    */
    const std::set<std::shared_ptr<Course>, pcourse_cmp>& get_courses() {
        return courses;
    }

    /*
        Dodaje kurs do zbioru kursów, na które student jest zapisany.
    */
    void add_course(std::shared_ptr<Course> course) { courses.insert(course); }
};

/*
    Klasa 'Teacher' reprezentująca nauczyciela na uczelni. Wirtualnie dziedziczy
    po klasie 'Person'.
*/

class Teacher : virtual public Person {
private:
    std::set<std::shared_ptr<Course>, pcourse_cmp> courses;

public:
    Teacher(const std::string& name, const std::string& surname)
        : Person(name, surname){};
    Teacher() = delete;
    virtual ~Teacher() = default;

    /*
        Zwraca referencję do zbioru kursów, które nauczyciel prowadzi.
    */
    const std::set<std::shared_ptr<Course>, pcourse_cmp>& get_courses() {
        return courses;
    }

    /*
        Dodaje kurs do zbioru kursów, które nauczyciel prowadzi.
    */
    void add_course(std::shared_ptr<Course> course) { courses.insert(course); }
};

/*
    Klasa 'PhDStudent' reprezentująca doktoranta na uczelni. Dziedziczy po
    klasie 'Student' oraz 'Teacher'.
*/

class PhDStudent : public Student, public Teacher {
public:
    PhDStudent(const std::string& name, const std::string& surname)
        : Person(name, surname),
          Student(name, surname, true),
          Teacher(name, surname) {}
    PhDStudent() = delete;
    virtual ~PhDStudent() = default;
};

/*
    Klasa 'College' reprezentująca uczelnię. Zawiera sety studentów,
    nauczycieli, doktorantów oraz mapę kursów na pary setów studentów i
    nauczycieli, którzy odpowiednio uczestniczą w kursie albo go nauczają.
*/

class College {
private:
    std::string NON_EXISTING_COURSE = "Non-existing course.";
    std::string NON_EXISTING_PERSON = "Non-existing person.";
    std::string INCORRECT_OPERATION_ON_INACTIVE_COURSE =
        "Incorrect operation on an inactive course.";
    std::string INCORRECT_OPERATION_FOR_INACTIVE_STUDENT =
        "Incorrect operation for an inactive student.";

    /*
        Funkcja pomocnicza, która tworzy string reprezentujący wyrażenie
        regularne na podstawie wzorca.
    */
    std::string make_regex_string(std::string pattern) {
        std::string regex_string = "";

        for (char c : pattern) {
            if (c != '?' && c != '*') {
                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                    regex_string += c;
                } else if (c >= '0' && c <= '9') {
                    regex_string += c;
                } else {
                    regex_string += "\\";
                    regex_string += c;
                }
            } else if (c == '*') {
                regex_string += ".*";
            } else if (c == '?') {
                regex_string += ".";
            }
        }

        return regex_string;
    }

    /*
        Struktura pomocnicza, która pozwala na porównywanie obiektów typu
        'Person' na podstawie nazwiska oraz imienia.
    */
    struct name_cmp {
        bool operator()(std::shared_ptr<Person> p1,
                        std::shared_ptr<Person> p2) const {
            if (p1->get_surname() > p2->get_surname()) {
                return false;
            } else if (p1->get_surname() < p2->get_surname()) {
                return true;
            } else if (p1->get_name() > p2->get_name()) {
                return false;
            } else if (p1->get_name() < p2->get_name()) {
                return true;
            } else {
                return false;
            }
        }
    };

    /*
        Struktura pomocnicza, która pozwala na porównywanie obiektów typu
        'Course' na podstawie nazwy.
    */
    struct course_cmp {
        bool operator()(std::shared_ptr<Course> p1,
                        std::shared_ptr<Course> p2) const {
            if (p1->get_name() > p2->get_name()) {
                return true;
            } else if (p1->get_name() < p2->get_name()) {
                return false;
            } else {
                return false;
            }
        }
    };

    /* Kontenery używane w implementacji. */

    std::set<std::shared_ptr<Student>, name_cmp> students;
    std::set<std::shared_ptr<Teacher>, name_cmp> teachers;
    std::set<std::shared_ptr<PhDStudent>, name_cmp> phd_students;
    std::map<std::shared_ptr<Course>,
             std::pair<std::set<std::shared_ptr<Student>, name_cmp>,
                       std::set<std::shared_ptr<Teacher>, name_cmp>>,
             course_cmp>
        courses;

    /*
        Funkcja pomocnicza, która sprawdza czy można dodać osobę o podanym
        imieniu oraz nazwisku do uczelni. Nie można jeżeli taka osoba już
        istnieje, czyli istnieje osoba o podanym imieniu i nazwisku.
    */

    bool can_insert(std::string name, std::string surname) {
        return !(
            students.contains(std::make_shared<Student>(name, surname)) ||
            teachers.contains(std::make_shared<Teacher>(name, surname)) ||
            phd_students.contains(std::make_shared<PhDStudent>(name, surname)));
    }

public:
    College() = default;

    /*
        Dodaje przedmiot. Zwraca true, jeśli przedmiot został dodany, a false,
        jeśli istnieje już przedmiot o tej samej nazwie.
    */

    bool add_course(const std::string& name, const bool active = true) {
        return courses
            .insert({std::make_shared<Course>(name, active),
                     {std::set<std::shared_ptr<Student>, name_cmp>(),
                      std::set<std::shared_ptr<Teacher>, name_cmp>()}})
            .second;
    }

    /*
        Dodaje studenta, doktoranta lub nauczyciela. Trzeci parametr jest
        uwzględniany tylko wtedy, gdy T nie jest Teacher. Zwraca true, jeśli
        osoba o podanych imieniu i nazwisku została dodana, a false, jeśli już
        istnieje taka osoba.
    */

    template <typename T>
    bool add_person(std::string name, std::string surname);

    template <typename T>
    bool add_person(std::string name, std::string surname, bool active);

    /*
        Zwraca wektor wskaźników do przedmiotów, których nazwa pasuje do wzorca.
    */

    auto find_courses(std::string pattern) {
        std::vector<std::shared_ptr<Course>> matching_courses;

        std::string regex_string = make_regex_string(pattern);

        std::regex searched(regex_string);
        std::cmatch match;

        for (auto& [course, students_teachers] : courses) {
            if (std::regex_match(course->get_name().c_str(), match, searched)) {
                matching_courses.insert(matching_courses.begin(), course);
            }
        }

        return matching_courses;
    }

    /*
        Zmienia status przedmiotu na aktywny lub nieaktywny zgodnie z podaną
        wartością. Zwraca true, jeśli przedmiot istnieje, a false w przeciwnym
        przypadku.
    */

    bool change_course_activeness(std::shared_ptr<Course> course, bool active) {
        if (courses.contains(course)) {
            course->change_activeness(active);
            return true;
        }

        return false;
    }

    /*
        Usuwa przedmiot ze zbioru. Zwraza true, jeśli przedmiot istniał, a false
        w przeciwnym przypadku.
    */

    bool remove_course(std::shared_ptr<Course> course) {
        if (courses.contains(course)) {
            course->change_activeness(false);
            courses.erase(course);
            return true;
        }

        return false;
    }

    /*
        Szuka czy istnieje student o podanym imieniu i nazwisku w secie
        studentów i doktorantów. Jeśli tak, to zmienia jego status aktywności
        zgodnie z podaną wartością. Zwraca true, jeśli student istnieje, a false
        w przeciwnym przypadku.
    */

    bool change_student_activeness(std::shared_ptr<Student> student,
                                   bool active) {
        if (students.contains(student)) {
            student->change_activeness(active);
            return true;
        } else if (phd_students.contains(std::make_shared<PhDStudent>(
                       student->get_name(), student->get_surname()))) {
            student->change_activeness(active);
            return true;
        }

        return false;
    }

    /*
        Wyszukuje studentów, nauczycieli lub doktorantów o podanym imieniu i
        nazwisku. Zwraca wektor wskaźników do znalezionych osób.
    */

    template <typename T>
    auto find(std::string name_pattern, std::string surname_pattern);

    template <typename T>
    auto find(std::shared_ptr<Course>) const;

    /*
        Przypisuje studenta, nauczyciela lub doktoranta do przedmiotu, czyli
        dodaje go do zbioru studentów lub nauczycieli, którzy uczestniczą w
        przedmiocie, oraz dodaje przedmiot do zbioru przedmiotów, na które jest
        zapisany student, nauczyciel lub doktorant. Zwraca true, jeśli operacja
        się powiodła, a false jeśli osoba była już zapisana na przedmiot. Rzuca
        wyjątek, jeśli osoba lub przedmiot nie istnieje, lub jeśli przedmiot lub
        student jest nieaktywny.
    */

    template <typename T>
    bool assign_course(std::shared_ptr<T> person,
                       std::shared_ptr<Course> course);
};

/* Implementacja metod klasy 'College'. */

template <>
auto College::find<Student>(std::shared_ptr<Course> course) const{
    auto crs_it = courses.find(course);

    if (crs_it == courses.end() || (crs_it->first).get() != course.get()) {
        return std::set<std::shared_ptr<Student>, College::name_cmp>();
    }
    return courses.at(course).first;
}

template <>
auto College::find<Teacher>(std::shared_ptr<Course> course) const{
        auto crs_it = courses.find(course);

    if (crs_it == courses.end() || (crs_it->first).get() != course.get()) {
        return std::set<std::shared_ptr<Teacher>, College::name_cmp>();
    }
    return courses.at(course).second;
}

template <>
bool College::add_person<Student>(std::string name, std::string surname,
                                  bool active) {
    if (can_insert(name, surname)) {
        return students.insert(std::make_shared<Student>(name, surname, active))
            .second;
    }
    return false;
}

template <>
bool College::add_person<Student>(std::string name, std::string surname) {
    if (can_insert(name, surname)) {
        return students.insert(std::make_shared<Student>(name, surname)).second;
    }
    return false;
}

template <>
bool College::add_person<PhDStudent>(std::string name, std::string surname) {
    if (can_insert(name, surname)) {
        return phd_students.insert(std::make_shared<PhDStudent>(name, surname))
            .second;
    }
    return false;
}

template <>
bool College::add_person<Teacher>(std::string name, std::string surname) {
    if (can_insert(name, surname)) {
        return teachers.insert(std::make_shared<Teacher>(name, surname)).second;
    }
    return false;
}

template <>
auto College::find<PhDStudent>(std::string name_pattern,
                               std::string surname_pattern) {
    std::string name_regex_string = make_regex_string(name_pattern);
    std::string surname_regex_string = make_regex_string(surname_pattern);

    std::vector<std::shared_ptr<PhDStudent>> matching_phd_students;

    /*
        Przeszukujemy set doktorantów i dodajemy do wektora te, które pasują do
        wzorca.
    */
    for (auto& phd_student : phd_students) {
        if (std::regex_match(phd_student->get_name().c_str(),
                             std::regex(name_regex_string)) &&
            std::regex_match(phd_student->get_surname().c_str(),
                             std::regex(surname_regex_string))) {
            matching_phd_students.insert(matching_phd_students.begin(),
                                         phd_student);
        }
    }

    return matching_phd_students;
}

template <>
auto College::find<Student>(std::string name_pattern,
                            std::string surname_pattern) {
    std::string name_regex_string = make_regex_string(name_pattern);
    std::string surname_regex_string = make_regex_string(surname_pattern);

    std::vector<std::shared_ptr<Student>> matching_students;

    /*
        Przeszukujemy set studentów i dodajemy do wektora te, które pasują do
        wzorca.
    */
    for (auto& student : students) {
        if (std::regex_match(student->get_name().c_str(),
                             std::regex(name_regex_string)) &&
            std::regex_match(student->get_surname().c_str(),
                             std::regex(surname_regex_string))) {
            matching_students.insert(matching_students.begin(), student);
        }
    }

    // Wywołujemy find dla doktorantów i dodajemy do wektora wynik.
    std::vector<std::shared_ptr<PhDStudent>> matching_PhDstudents =
        find<PhDStudent>(name_pattern, surname_pattern);

    matching_students.insert(matching_students.end(),
                             matching_PhDstudents.begin(),
                             matching_PhDstudents.end());

    // Sortujemy wektor wynikowy.
    std::sort(matching_students.begin(), matching_students.end(), name_cmp());

    return matching_students;
}

template <>
auto College::find<Teacher>(std::string name_pattern,
                            std::string surname_pattern) {
    std::string name_regex_string = make_regex_string(name_pattern);
    std::string surname_regex_string = make_regex_string(surname_pattern);

    std::vector<std::shared_ptr<Teacher>> matching_teachers;

    /*
        Przeszukujemy set nauczycieli i dodajemy do wektora te, które pasują do
        wzorca.
    */
    for (auto& teacher : teachers) {
        if (std::regex_match(teacher->get_name().c_str(),
                             std::regex(name_regex_string)) &&
            std::regex_match(teacher->get_surname().c_str(),
                             std::regex(surname_regex_string))) {
            matching_teachers.insert(matching_teachers.begin(), teacher);
        }
    }

    // Wywołujemy find dla doktorantów i dodajemy do wektora wynik.
    std::vector<std::shared_ptr<PhDStudent>> matching_PhDstudents =
        find<PhDStudent>(name_pattern, surname_pattern);

    matching_teachers.insert(matching_teachers.end(),
                             matching_PhDstudents.begin(),
                             matching_PhDstudents.end());

    // Sortujemy wektor wynikowy.
    std::sort(matching_teachers.begin(), matching_teachers.end(), name_cmp());

    return matching_teachers;
}

template <>
auto College::find<Person>(std::string name_pattern,
                           std::string surname_pattern) {
    std::vector<std::shared_ptr<Person>> matching_people;

    std::string name_regex_string = make_regex_string(name_pattern);
    std::string surname_regex_string = make_regex_string(surname_pattern);

    /*
        Znajdujemy studentów i doktorantów których imię i nazwsko pasują do
        wzorca i dodajemy ich do wektora wynik.
    */
    std::vector<std::shared_ptr<Student>> matching_students =
        find<Student>(name_pattern, surname_pattern);

    matching_people.insert(matching_people.end(), matching_students.begin(),
                           matching_students.end());

    /*
        Znajdujemy nauczycieli których imię i nazwsko pasują do wzorca i
        dodajemy ich do wektora wynik. Nie możemy wywołać find dla nauczycieli,
        bo wtedy znajdą się w wyniku również doktoranci, którzy już są zawarci w
        wektorze wynikowym z powodu wywołania find dla studentów.
    */
    for (auto& teacher : teachers) {
        if (std::regex_match(teacher->get_name().c_str(),
                             std::regex(name_regex_string)) &&
            std::regex_match(teacher->get_surname().c_str(),
                             std::regex(surname_regex_string))) {
            matching_people.insert(matching_people.begin(), teacher);
        }
    }

    std::sort(matching_people.begin(), matching_people.end(), name_cmp());

    return matching_people;
}

template <>
bool College::assign_course<Teacher>(std::shared_ptr<Teacher> person,
                                     std::shared_ptr<Course> course) {
    auto crs_it = courses.find(course);

    // Rzucamy wyjątek, jeśli kurs nie istnieje.
    if (crs_it == courses.end() || (crs_it->first).get() != course.get()) {
        throw std::invalid_argument(NON_EXISTING_COURSE);
    }

    // Sprawdzamy, czy nauczyciel już nie jest przypisany do kursu.
    auto p_it = courses.at(course).second.find(person);

    if (p_it != courses.at(course).second.end() &&
        (*p_it).get() == person.get()) {
        return false;
    }

    auto t_it = teachers.find(person);
    auto phd_it = phd_students.find(std::make_shared<PhDStudent>(
        person->get_name(), person->get_surname()));

    /*
        Rzucamy wyjątek jeżeli nie ma takiego nauczyciela ani takiego doktoranta
        lub jeżeli kurs jest nieaktywny.
    */
    if ((t_it == teachers.end() || (*t_it).get() != person.get()) &&
        (phd_it == phd_students.end() || (*phd_it).get() != person.get())) {
        throw std::invalid_argument(NON_EXISTING_PERSON);
    } else if (!course->is_active()) {
        throw std::invalid_argument(INCORRECT_OPERATION_ON_INACTIVE_COURSE);
    }

    person->add_course(course);
    courses.at(course).second.insert(person);

    return true;
}

template <>
bool College::assign_course<Student>(std::shared_ptr<Student> person,
                                     std::shared_ptr<Course> course) {
    auto crs_it = courses.find(course);

    // Rzucamy wyjątek, jeśli kurs nie istnieje.
    if (crs_it == courses.end() || (crs_it->first).get() != course.get()) {
        throw std::invalid_argument(NON_EXISTING_COURSE);
    }

    // Sprawdzamy, czy student już nie jest przypisany do kursu.
    auto p_it = courses.at(course).first.find(person);

    if (p_it != courses.at(course).first.end() &&
        (*p_it).get() == person.get()) {
        return false;
    }

    auto s_it = students.find(person);
    auto phd_it = phd_students.find(std::make_shared<PhDStudent>(
        person->get_name(), person->get_surname()));

    /*
        Rzucamy wyjątek jeżeli nie ma takiego nauczyciela ani takiego doktoranta
        lub jeżeli kurs jest nieaktywny.
    */
    if ((s_it == students.end() || (*s_it).get() != person.get()) &&
        (phd_it == phd_students.end() || (*phd_it).get() != person.get())) {
        throw std::invalid_argument(NON_EXISTING_PERSON);
    } else if (!course->is_active()) {
        throw std::invalid_argument(INCORRECT_OPERATION_ON_INACTIVE_COURSE);
    } else if (!person->is_active()) {
        throw std::invalid_argument(INCORRECT_OPERATION_FOR_INACTIVE_STUDENT);
    }

    (*person).add_course(course);
    courses.at(course).first.insert(person);

    return true;
}

#endif

