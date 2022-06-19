//
// Created by Ben on 6/19/22.
//

#ifndef HEAPMANAGER_GLTHREAD_H
#define HEAPMANAGER_GLTHREAD_H

typedef struct _glthread{
    struct _glthread *left;
    struct _glthread *right;
} glthread_t

void glthread_priority_insert(glthread_t*, glthread_t*, int (*comp_fn)(void *, void *), int );
void glthread_add_before(glthread_t*, glthread_t*);
unsigned int get_glthread_list_count(glthread_t*);
void glthread_add_last(glthread_t*, glthread_t*);
void glthread_add_next(glthread_t*, glthread_t*);
void delete_glthread_list(glthread_t*);
void remove_glthread(glthread_t*);
void init_glthread(glthread_t*);

#define IS_GLTHREAD_LIST_EMPTY(glthreadptr)         \
    ((glthreadptr)->right == 0 && (glthreadptr)->left == 0)

#define GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name, glthreadptr)           \
    static inline structure_name * fn_name(glthread_t *glthreadptr){                   \
        return (structure_name *)((char *)(glthreadptr) - (char *)&(((structure_name *)0)->field_name)); \
    }
#define BASE(glthreadptr)   ((glthreadptr)->right)

#define ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr)                                      \
{                                                                                                  \
    glthread_t *_glthread_ptr = NULL;                                                              \
    glthreadptr = BASE(glthreadptrstart);                                                          \
    for(; glthreadptr!= NULL; glthreadptr = _glthread_ptr){                                        \
        _glthread_ptr = (glthreadptr)->right;

#define ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr)                                        \
        }}

#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr, offset)  \
    (void *)((char *)(glthreadptr) - offset)


#endif
