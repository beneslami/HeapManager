#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mm.h"
static vm_page_for_families_t  *first_vm_page_for_families = NULL;
static size_t SYSTEM_PAGE_SIZE = 0;

void mm_init(){
    SYSTEM_PAGE_SIZE = getpagesize(); // will return of memory page size
} // initialization of memory manager

static void *mm_get_new_vm_page_from_kernel(int units){
    char *vm_page = mmap(0, units*SYSTEM_PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, 0, 0);
    if(vm_page == MAP_FAILED){
        printf("Error vm page allocation\n");
        return NULL;
    }
    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);
    return (void*)vm_page;
}

static void mm_return_vm_page_to_kernel(void *vm_page, int units){
    if(munmap(vm_page, units*SYSTEM_PAGE_SIZE)){
        printf("Error deallocating vm page\n");
    }
}

void mm_instantiate_new_page_family(char *struct_name, uint32_t struct_size){
    vm_page_family_t *vm_page_family_curr = NULL;
    vm_page_for_families_t *new_vm_page_for_families = NULL;
    if(struct_size > SYSTEM_PAGE_SIZE){
        printf("ERROR: %s structure %s size exceeds system page size\n", __FUNCTION__ , struct_name);
        return;
    }
    if(!first_vm_page_for_families){
        first_vm_page_for_families = (vm_page_for_families_t*) mm_get_new_vm_page_from_kernel(1);
        first_vm_page_for_families->next = NULL;
        strncpy(first_vm_page_for_families->vm_page_family[0].struct_name, struct_name, MM_MAX_STRUCT_NAME);
        first_vm_page_for_families->vm_page_family[0].struct_size = struct_size;
        return;
    }
    uint32_t count = 0;
    ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families, vm_page_family_curr){
        if(strncmp(vm_page_family_curr->struct_name, struct_name, MM_MAX_STRUCT_NAME) != 0){
            count++;
            continue;
        }
        assert(0);
    }ITERATE_PAGE_FAMILIES_END(first_vm_page_for_families, vm_page_family_curr);
    if(count == MAX_FAMILIES_PER_VM_PAGE){
        new_vm_page_for_families = (vm_page_for_families_t*) mm_get_new_vm_page_from_kernel(1);
        new_vm_page_for_families->next = first_vm_page_for_families;
        first_vm_page_for_families = new_vm_page_for_families;
        vm_page_family_curr = &first_vm_page_for_families->vm_page_family[0];
    }

    strncpy(vm_page_family_curr->struct_name, struct_name, MM_MAX_STRUCT_NAME);
    vm_page_family_curr->struct_size = struct_size;
    vm_page_family_curr->first_page = NULL;
}

vm_bool_t mm_is_vm_page_empty(vm_page_t *vm_page){
    if(vm_page->block_meta_data.next_block == NULL &&
       vm_page->block_meta_data.prev_block == NULL &&
       vm_page->block_meta_data.is_free == MM_TRUE){

        return MM_TRUE;
    }
    return MM_FALSE;
}

static inline uint32_t mm_max_page_allocatable_memory(int units){
    return (uint32_t)
            ((SYSTEM_PAGE_SIZE * units) - offset_of(vm_page_t, page_memory));
}

#define MAX_PAGE_ALLOCATABLE_MEMORY(units) (mm_max_page_allocatable_memory(units))

vm_page_t *
allocate_vm_page(vm_page_family_t *vm_page_family, int units){

    vm_page_t *prev_page =
            mm_get_available_page_index(vm_page_family);

    vm_page_t *vm_page = mm_get_new_vm_page_from_kernel(units);
    vm_page->block_meta_data.is_free = MM_TRUE;
    vm_page->block_meta_data.block_size =
            MAX_PAGE_ALLOCATABLE_MEMORY(units);
    vm_page->block_meta_data.offset =
            offset_of(vm_page_t, block_meta_data);
    init_glthread(&vm_page->block_meta_data.priority_thread_glue);
    vm_page->block_meta_data.prev_block = NULL;
    vm_page->block_meta_data.next_block = NULL;
    vm_page->next = NULL;
    vm_page->prev = NULL;
    vm_page_family->no_of_system_calls_to_alloc_dealloc_vm_pages++;
    vm_page->pg_family = vm_page_family;

    if(!prev_page){
        vm_page->page_index = 0;
        vm_page->next = vm_page_family->first_page;
        if(vm_page_family->first_page)
            vm_page_family->first_page->prev = vm_page;
        vm_page_family->first_page = vm_page;
        return vm_page;
    }

    vm_page->next = prev_page->next;
    vm_page->prev = prev_page;
    if(vm_page->next)
        vm_page->next->prev = vm_page;
    prev_page->next = vm_page;
    vm_page->page_index = prev_page->page_index + 1;
    return vm_page;
}

void
mm_vm_page_delete_and_free(
        vm_page_t *vm_page){

    vm_page_family_t *vm_page_family =
            vm_page->pg_family;

    assert(vm_page_family->first_page);

    if(vm_page_family->first_page == vm_page){
        vm_page_family->first_page = vm_page->next;
        if(vm_page->next)
            vm_page->next->prev = NULL;
        vm_page_family->no_of_system_calls_to_alloc_dealloc_vm_pages++;
        vm_page->next = NULL;
        vm_page->prev = NULL;
        mm_return_vm_page_to_kernel((void *)vm_page,
                                    vm_page->page_size / SYSTEM_PAGE_SIZE);
        return;
    }

    if(vm_page->next)
        vm_page->next->prev = vm_page->prev;
    vm_page->prev->next = vm_page->next;
    vm_page_family->no_of_system_calls_to_alloc_dealloc_vm_pages++;
    mm_return_vm_page_to_kernel((void *)vm_page, vm_page->page_size / SYSTEM_PAGE_SIZE);
}