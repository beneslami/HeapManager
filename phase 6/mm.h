//
// Created by Ben on 6/17/22.
//

#ifndef HEAPMANAGER_MM_H
#define HEAPMANAGER_MM_H

#include <unistd.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "glthread.h"

#define MM_MAX_STRUCT_NAME 32

typedef enum{
    MM_FALSE,
    MM_TRUE
} vm_bool_t;

typedef struct block_meta_data_{

    vm_bool_t is_free;
    uint32_t block_size;
    uint32_t offset;    /*offset from the start of the page*/
    glthread_t priority_thread_glue;
    struct block_meta_data_ *prev_block;
    struct block_meta_data_ *next_block;
} block_meta_data_t;
GLTHREAD_TO_STRUCT(glthread_to_block_meta_data,
                   block_meta_data_t, priority_thread_glue, glthread_ptr);

#define offset_of(container_structure, field_name)  \
    ((size_t)&(((container_structure *)0)->field_name))

typedef struct vm_page_family_{
    char struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
    vm_page_t *first_page;
    glthread_t free_block_priority_list_head;
}vm_page_family_t;

typedef struct vm_page_for_families_{
    struct vm_page_for_families_ *next;
    vm_page_family_t vm_page_family[0];
}vm_page_for_families_t;

typedef struct vm_page_{
    struct vm_page_ *next;
    struct vm_page_ *prev;
    struct vm_page_family_ *pg_family; /*back pointer*/
    uint32_t page_index;
    uint32_t page_size;
    block_meta_data_t block_meta_data;
    char page_memory[0];
} vm_page_t;

#define MAX_FAMILIES_PER_VM_PAGE \
    (SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t*) / \
    sizeof(vm_page_family_t))

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr, curr) \
{                                                                   \
    uint32_t count = 0;                                             \
    for(curr = (vm_page_family_t *)&vm_page_for_families_ptr->vm_page_family[0]; \
        curr->struct_size && count < MAX_FAMILIES_PER_VM_PAGE;      \
        curr++, count++){
#define ITERATE_PAGE_FAMILIES_END(vm_page_for_families_ptr, curr)  }}

#define MM_REG_STRUCT(struct_name) \
    (mm_instantiate_new_page_family(#struct_name, sizeof(struct_name)))


void mm_init();
static void *mm_get_new_vm_page_from_kernel(int);
static void mm_return_vm_page_to_kernel(void *, int);
void mm_instantiate_new_page_family(char*, uint32_t);
vm_bool_t mm_is_vm_page_empty(vm_page_t *vm_page);
vm_page_t *allocate_vm_page();
void mm_vm_page_delete_and_free(vm_page_t *vm_page);

#endif
