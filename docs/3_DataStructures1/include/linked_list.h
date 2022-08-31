#ifndef _LIST_H
#define _LIST_H

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief structure of a list node
 *
 * @param position is the position in the list relevant to the head (1)
 * @param data void pointer to whatever data that list points to
 * @param prev pointer to the node before it
 * @param next pointer to the node after it
 */
typedef struct list_node_t
{
    uint32_t position;
    void *data;
    struct list_node_t *next;
} list_node_t;

/**
 * @brief A pointer to a user-defined free function.  This is used to free
 *        memory allocated for list data.  For simple data types, this
 * 		  is just a pointer to the standard free function.  More complex
 * structs stored in lists may require a function that calls free on multiple
 * components.
 *
 */
typedef void (*FREE_F)(void *);

/**
 * @brief A pointer to a user-defined function for comparing que values to a
 *        search value and returning the node position of that value
 *
 */
typedef void *(*CMP_F)(const void *, const void *);

/**
 * @brief A pointer to a user-defined function that gets called in the
 *        foreach_call
 *        on each item in the linked list.
 *
 */
typedef void (*ACT_F)(void *);

/**
 * @brief structure of a list object
 *
 * @param size is the number of nodes the list is currently storing
 * @param head pointer to the head node
 * @param tail pointer to the tail node
 * @param customfree pointer to the user defined free function
 * @param compare_function pointer to the user defined compare function
 */
typedef struct list_t
{
    uint32_t size;
    list_node_t *head;
    list_node_t *tail;
    FREE_F customfree;
    CMP_F compare_function;
} list_t;

/**
 * @brief Default Compare Function. Should be used if NULL specified for cmp_fun
 * in createion
 *
 * @param value_to_find
 * @param node
 * @return void*
 */
void *default_compare(void *value_to_find, void *node);

/**
 * @brief creates a new list
 *
 * @param customfree pointer to the free function to be used with that list
 * @param compare_function pointer to the compare function to be used with that
 *        list. must
 * @returns pointer to allocated list on success or NULL on failure
 */
list_t *list_new(FREE_F, CMP_F);

/**
 * @brief pushes a new node onto the head of list
 *
 * @param list list to push the node into
 * @param data data to be pushed into node
 * @returns 0 on success, non-zero value on failure
 */
int list_push_head(list_t *list, void *data);

/**
 * @brief pushes a new node onto the tail of list
 *
 * @param list list to push the node into
 * @param data data to be pushed into node
 * @return 0 on success, non-zero value on failure
 */
int list_push_tail(list_t *list, void *data);

/**
 * @brief checks if the list object is empty
 *
 * @param list pointer to linked list object to be checked
 * @returns non-zero if list is empty, 0 value if not empty
 */
int list_emptycheck(list_t *list);

/**
 * @brief pops the head node out of the list
 *
 * @param list list to pop the node out of
 * @return pointer to popped node on success, NULL on failure
 */
list_node_t *list_pop_head(list_t *list);

/**
 * @brief pops the tail node out of the list
 *
 * @param list list to pop the node out of
 * @return pointer to popped node on success, NULL on failure
 */
list_node_t *list_pop_tail(list_t *list);

/**
 * @brief get the data from the node at the head of the list without
 *        popping
 *
 * @param list list to pop the node out of
 * @return pointer to head node on success, NULL on failure
 */
list_node_t *list_peek_head(list_t *list);

/**
 * @brief get the data from the node at the tail of the list without
 *        popping
 *
 * @param list list to pop the node out of
 * @return pointer to tail node on success, NULL on failure
 */
list_node_t *list_peek_tail(list_t *list);

/**
 * @brief remove a specific node from the list based on the data stored
 *        in that node
 *
 * @param list list to remove the node from
 * @param item_to_remove the data object to be searched for
 * @return 0 on success, non-zero value on failure
 */
int list_remove(list_t *list, void **item_to_remove);

/**
 * @brief perform a user defined action on the data contained in all of the
 *        nodes in list
 *
 * @param list list to perform actions on
 * @param action_function pointer to user defined action function
 * @return 0 on success, non-zero value on failure
 */
int list_foreach_call(list_t *list, ACT_F action_function);

/**
 * @brief find the first occurrance of a node containing the search_data as
 *        found by the user defined compare function
 *
 * @param list list to search through
 * @param search_data is the pointer to the address of the data to be searched
 *                    for
 * @return pointer to node found on success, NULL on failure
 */
list_node_t *list_find_first_occurrence(list_t *list, void **search_data);

/**
 * @brief find the first occurrance of a node containing the search_data as
 *        found by the user defined compare function
 *
 * @param list list to search through
 * @param receiving_list pointer to receiving list
 * @param search_data is the pointer to the address of the data to be searched
 *                    for
 * @return pointer to list of all found occurrecnces on success, NULL on failure
 */
list_t *list_find_all_occurrences(list_t *list, void **search_data);

/**
 * @brief sort list as per user defined compare function
 *
 * @param list pointer to list to be sorted
 * @return 0 on success, non-zero value on failure
 */
int list_sort(list_t *list);

/**
 * @brief clear all nodes out of a list
 *
 * @param list list to clear out
 * @return 0 on success, non-zero value on failure
 */
int list_clear(list_t *list);

/**
 * @brief delete a list
 *
 * @param list_address pointer to list pointer
 * @return 0 on success, non-zero value on failure
 */
int list_delete(list_t **list_address);

/**
 * @brief frees an item and its associated memory
 *
 * @param mem_addr pointer of the item to be free'd
 */
void custom_free(void *mem_addr);

#endif