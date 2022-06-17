//
// Created by Ben on 6/17/22.
//

#ifndef HEAPMANAGER_MM_H
#define HEAPMANAGER_MM_H

#include <unistd.h>
#include <assert.h>

#define MM_MAX_STRUCT_NAME 32
void mm_init();
static void *mm_get_new_vm_page_from_kernel(int);
static void mm_return_vm_page_to_kernel(void *, int);
void mm_instantiate_new_page_family(char*, uint32_t);


typedef struct vm_page_family_{
    char struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
}vm_page_family_t;

typedef struct vm_page_for_families_{
    struct vm_page_for_families_ *next;
    vm_page_family_t vm_page_family[0];
}vm_page_for_families_t;

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


#endif
