## Stack Container Implementation

Language: C++

Class: Kurs programowania w C++ (Programming in C++)

This assignment focuses on implementing a container template that behaves like a stack, where each element consists of a key and a value. It provides methods of a standard stack such as pop(), front() and push(element), but also provides additional methods such as:

void pop(K const &); - Removes the last added element with the given key

V & front(K const &);
V const & front(K const &) const; - Single-parameter front methods return a reference to the value of the element with the given key closest to the top of the stack. In the non-const version, the returned reference should allow modifying the value. Modifying the stack may invalidate the returned reference

size_t count(K const &) const; - Returns the number of elements on the stack with the specified key

It also allows the user to access the structure using an iterator that satisfies the std::forward_iterator concept.

The stack class should be exception-transparent, meaning it should propagate any exceptions thrown by the functions it calls and operations on its members. The observable state of the object should not change if modifying operations fail, particularly, they should not invalidate iterators.


