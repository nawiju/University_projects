## College
Language: C++

Class: Kurs programowania w C++ (Programming in C++)


The task requires designing and implementing classes for managing a university in a simplified form. The following classes are expected:

    Course

        Represents a subject with a name and a boolean attribute indicating whether the course is active.

        Constructor with parameters: name, activity (default value is true).

        Methods: get_name() and is_active().

    Person

        Represents an individual with a first name and a last name.

        Constructor with parameters: first name, last name.

        Methods: get_name() and get_surname().

    Student

        Represents a student, inheriting from Person.

        Has a list of courses and an active status.

        Constructor with parameters: first name, last name, activity (default value is true).

        Methods: is_active(), get_courses().

    Teacher

        Represents a teacher, inheriting from Person.

        Has a list of courses taught.

        Constructor with parameters: first name, last name.

        Method: get_courses().

    PhDStudent
        Represents a Ph.D. student, simultaneously a student and a teacher.

    College

        Represents the university, containing collections of persons and courses.

        Public methods:
            bool add_course(name, active = true)
            auto find_courses(pattern)
            bool change_course_activeness(course, active)
            bool remove_course(course)
            template <typename T> bool add_person(name, surname, active = true)
            bool change_student_activeness(student, active)
            template <typename T> auto find(name_pattern, surname_pattern)
            template <typename T> auto find(course)
            template <typename T> bool assign_course(person, course)
          
