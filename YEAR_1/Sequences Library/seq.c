#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "seq.h"

typedef struct seq seq_t;
typedef struct classes class_t;

struct seq {
    seq_t *next_number[3];
    seq_t *next;
    seq_t *prev;
    bool root;
    class_t *abstract_class;
};

struct classes {
    char *name;
    int size;
    seq_t *beginning;
    seq_t *end; 
};

// Tworzy szkielet node'a w drzewie
void make_node(bool root, seq_t **node) {
    (*node)->root = root; 
    (*node)->prev = NULL; 
    (*node)->next = NULL;

    for (int i = 0; i < 3; i++) 
        (*node)->next_number[i] = NULL; 
} 

void create_abstract_class(seq_t *node) {
    node->abstract_class->name = NULL; 
    node->abstract_class->size = 1; 
    node->abstract_class->beginning = node;
    node->abstract_class->end = node;
}

seq_t *create_tree_node(seq_t **parent, int i) {
    seq_t *new_node = malloc(sizeof(struct seq));

    if (new_node == NULL) { 
        free(new_node);
        return NULL; 
    }

    make_node(false, &new_node);

    new_node->abstract_class = malloc(sizeof(struct classes));

    if (new_node->abstract_class == NULL) { 
        free(new_node->abstract_class);
        free(new_node);
        return NULL; 
    }

    create_abstract_class(new_node);

    (*parent)->next_number[i] = new_node; 

    return new_node; 
}

seq_t *seq_new(void) {
    seq_t *root = malloc(sizeof(struct seq));

    if (root == NULL) { 
        free(root);
        errno = ENOMEM;
        return NULL;
    }

    make_node(true, &root);

    root->abstract_class = NULL; 

    return root; 
}

// Sprawdza czy ciąg jest poprawny według polecenia zadania
bool string_valid(char const *s) {
    if (s != NULL && (int)strlen(s) > 0) {
        int i = 0; 

        while (i < (int)strlen(s)) {
            if (s[i] != '0' && s[i] != '1' && s[i] != '2')  
                return false; 
            i++;
        }

        if (s[i] != '\0')  
            return false; 

        return true; 
    }
    
    return false;
}

bool name_valid(char const *n) {
    return (n != NULL && (int)strlen(n) > 0 && n[(int)strlen(n)] == '\0');
}

// Sprawdza czy ciągi są identyczne
bool identical_strings(char const *n, char const *s) {
    if (n == NULL || s == NULL) 
        return false; 

    if ((int)strlen(s) != (int)strlen(n)) 
        return false; 
    
    for (int i = 0; i < (int)strlen(s); i++) 
        if (s[i] != n[i]) 
            return false;

    return true; 
}

// Znajduje szukany ciąg w drzewie, zmienia pointer p na ostatni node w ciągu
// i = -1 jeżeli ciąg nie znajduje się w drzewie
void find_seq(int *i, char const *s, seq_t **p, bool shorter) {
    int length = (int)strlen(s);

    if (shorter == true) 
        length--; 

    while ((*i) < length && (*i) >= 0) {
        if ((*p)->next_number[(int)s[*i] - '0'] != NULL) 
            (*p) = (*p)->next_number[(int)s[*i] - '0']; 
        else  
            (*i) = -2; 
            
        (*i)++;
    }   
} 

// Przy usuwaniu node'ów z drzewa, poprawia kolejność node'ów w liście do
// której node należy
void adjust_node_list_order (seq_t *p) {
  
    if (p->next != NULL || p->prev != NULL) {
        // Środkowy node listy
        if (p->next != NULL && p->prev != NULL) {
            p->prev->next = p->next; 
            p->next->prev = p->prev;
        } 
        // Końcowy node listy
        else if (p->next == NULL) { 
            p->prev->next = NULL; 
            p->abstract_class->end = p->prev; 
        } 
        // Początkowy node listy
        else if (p->prev == NULL) { 
            p->next->prev = NULL;
            p->abstract_class->beginning = p->next; 
        } 
    }
}

void delete_abstract_class (class_t *abstract_class) {
    if (abstract_class->name != NULL) 
        free(abstract_class->name); 
                
    free(abstract_class); 
}

void seq_delete(seq_t *p) {
    
    if (p != NULL) {
        for (int i = 0; i < 3; i++) 
            seq_delete(p->next_number[i]);

        if (p->root == false) {
            p->abstract_class->size -= 1; 
    
            adjust_node_list_order (p);

            if (p->abstract_class->size == 0) 
                delete_abstract_class(p->abstract_class);
        }

        free(p);
    }
}

int seq_remove(seq_t *p, char const *s) {
   
    if (p == NULL || string_valid(s) == false) {
        errno = EINVAL;
        return -1;
    }

    int i = 0, j = 0;
    int length = (int)strlen(s);
    seq_t *pointer = p, *pointer_help = p;

    find_seq(&i, s, &pointer, false);

    if (i == length) {
        find_seq(&j, s, &pointer_help, true);

        pointer_help->next_number[(int)s[i-1] - '0'] = NULL;

        seq_delete(pointer);

        return 1;
    }

    return 0; 
} 

int seq_valid(seq_t *p, char const *s) {
  
    if (string_valid(s) && p != NULL) {
        int i = 0; 
        seq_t *pointer = p;
     
        find_seq(&i, s, &pointer, false);  

        if (i == -1)
            return 0;
        else  
            return 1;
    }

    errno = EINVAL;
    return -1;
}

// Usuwa node'y które zostały stworzone przed tym jak nastąpił błąd np. 
// alokacji pamięci
void delete_created_nodes(seq_t *p, int last_index, char const *s) {
    seq_delete(p->next_number[(int)s[last_index] - '0']);
    p->next_number[(int)s[last_index] - '0'] = NULL; 
}

int seq_add(seq_t *p, char const *s) {
    int value = 0;

    if (p == NULL || string_valid(s) == false) { 
        errno = EINVAL;
        return -1;
    }

    int last_index = 0;
    seq_t * pointer = p;
    seq_t * last_node = p; 

    for (int i = 0; i < (int)strlen(s); i++) {

        if (pointer->next_number[(int)s[i] - '0'] != NULL) {
            pointer = pointer->next_number[(int)s[i] - '0']; 
            last_node = pointer; 
            last_index = i;
        } 
        else {
            seq_t * new_node = create_tree_node(&pointer, (int)s[i] - '0');
            pointer = new_node;

            if (pointer == NULL) {
                delete_created_nodes(last_node, last_index, s);

                errno = ENOMEM;
                return -1;
            } 
            
            value = 1;
        }
    }

    return value; 
}

int seq_set_name(seq_t *p, char const *s, char const *n) {

    if (p == NULL || string_valid(s) == false || name_valid(n) == false) {   
        errno = EINVAL;
        return -1; 
    }

    int i = 0; 
    seq_t *pointer = p;

    find_seq(&i, s, &pointer, false);

    if (i == -1 || 
    identical_strings(pointer->abstract_class->name, n) == true)  
        return 0; 

    free(pointer->abstract_class->name); 

    pointer->abstract_class->name = calloc((int)strlen(n) + 1, sizeof(char)); 

    if (pointer->abstract_class->name == NULL) {
        errno = ENOMEM;
        return -1;
    }

    strcpy(pointer->abstract_class->name, n);

    if (pointer->abstract_class->name == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    return 1; 
}

char const *seq_get_name(seq_t *p, char const *s) {

    if (p == NULL || string_valid(s) == false) {   
        errno = EINVAL;
        return NULL; 
    }

    int i = 0; 
    seq_t *pointer = p;

    find_seq(&i, s, &pointer, false);

    if (i == -1 || pointer->abstract_class->name == NULL) {    
        errno = 0;
        return NULL;
    } 
    else {
        return pointer->abstract_class->name; 
    }
}

// Determinuje która klasa abstrakcji jest większa bądź równa i zmienia 
// pointery larger i smaller odpowiednio
void set_class_hierarchy(seq_t **larger, seq_t **smaller, 
seq_t *pointer1, seq_t *pointer2) {
    if (pointer1->abstract_class->size >= pointer2->abstract_class->size) {
        *larger = pointer1->abstract_class->beginning;
        *smaller = pointer2->abstract_class->beginning;
    } else {
        *larger = pointer2->abstract_class->beginning;
        *smaller = pointer1->abstract_class->beginning;
    }
}

void merge_classes(seq_t *larger, seq_t *smaller) {
    larger->abstract_class->beginning = smaller->abstract_class->beginning;
    larger->abstract_class->size += smaller->abstract_class->size;

    while (smaller != smaller->abstract_class->end) {
        smaller->abstract_class = larger->abstract_class;
        smaller = smaller->next; 
    }
    
    char *name_smaller = smaller->abstract_class->name;

    free(name_smaller);
        
    free(smaller->abstract_class);

    smaller->abstract_class = larger->abstract_class;

    smaller->next = larger;
    larger->prev = smaller;
}

char *merge_names(char *name_s2, char *name_s1) {
   
    char *new_name = calloc((int)strlen(name_s1) + 
            (int)strlen(name_s2) + 1, sizeof(char));

    strcpy(new_name, name_s1);
    free(name_s1);
    strcat(new_name, name_s2);
    return new_name;
}

int seq_equiv(seq_t *p, char const *s1, char const *s2) {

    if (p == NULL || string_valid(s1) == false || 
        string_valid(s2) == false) {      
        errno = EINVAL;
        return -1;     
    }

    int i1 = 0, i2 = 0;
    seq_t *pointer1 = p, *pointer2 = p;
    seq_t *larger, *smaller;

    find_seq(&i1, s1, &pointer1, false);

    if (i1 != -1) 
        find_seq(&i2, s2, &pointer2, false);

    if (i1 == -1 || i2 == -1)  
        return 0;

    if (pointer1->abstract_class == pointer2->abstract_class) 
        return 0; 

    set_class_hierarchy(&larger, &smaller, pointer1, pointer2);

    char *name_smaller = smaller->abstract_class->name;
    char *name_larger = larger->abstract_class->name; 
        
    if (name_larger == NULL && name_smaller != NULL) {
        name_larger = calloc((int)strlen(name_smaller) + 1, sizeof(char)); 

        if (name_larger == NULL) { 
            errno = ENOMEM;
            return -1;
        }

        strcpy(name_larger, name_smaller);  

        if (name_larger == NULL) {
            errno = ENOMEM;
            return -1;
        }
    } 
    else if (name_larger != NULL && name_smaller != NULL &&
        identical_strings(name_larger, name_smaller) == false) {

        char *name_s1 = pointer1->abstract_class->name;
        char *name_s2 = pointer2->abstract_class->name;

        larger->abstract_class->name = merge_names(name_s2, name_s1);

        if (larger->abstract_class->name == NULL) { 
            errno = ENOMEM;
            return -1;
        }
    }

    merge_classes(larger, smaller);
    
    return 1; 
} 
