## Sequences Library

Language: C

Class: Architektura komputerów i systemy operacyjne (Computer architecture and operating systems)

This is an implementation of a dynamically loaded C library that handles sets of sequences with an equivalence relation. Sequences are represented as non-empty strings of numbers 0, 1, and 2. The library provides functions to create, delete, add, remove, and check the validity of sequences in a set. Additionally, the library allows setting or changing the name of an abstraction class to which a sequence belongs, getting the name of the abstraction class, and combining two equivalence classes into one.

The interface of the library is as follows:
    typedef struct seq seq_t;
    seq_t *seq_new(void); // Creates a new empty set of sequences.
    void seq_delete(seq_t *p); // Deletes a set of sequences and frees associated memory.
    int seq_add(seq_t *p, char const *s); // Adds a sequence and all non-empty prefixes of it to the set.
    int seq_remove(seq_t *p, char const *s); // Removes a sequence and all sequences for which it is a prefix from the set.
    int seq_valid(seq_t *p, char const *s); // Checks if a given sequence belongs to the set of sequences.
    int seq_set_name(seq_t *p, char const *s, char const *n); // Sets or changes the name of the abstraction class for a given sequence.
    char const *seq_get_name(seq_t *p, char const *s); // Gets the name of the abstraction class for a given sequence.
    int seq_equiv(seq_t *p, char const *s1, char const *s2); // Combines two abstraction classes into one.

The library handles errors such as invalid parameters and memory allocation failures.

