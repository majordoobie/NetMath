#ifndef _QUEUE_P_H
#define _QUEUE_P_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief structure of a queue node
 *
 * @param data      void pointer to whatever data that queue points to
 * @param priority  priority to add to q; higher priority gets preference in q
 */
typedef struct queue_p_node_t
{
    void *data;
    int priority;
} queue_p_node_t;

/**
 * @brief A pointer to a user-defined free function.  This is used to free
 *        memory allocated for queue data.  For simple data types, this is
 *        just a pointer to the standard free function.  More complex structs
 *        stored in queues may require a function that calls free on multiple
 *        components.
 *
 */
typedef void (*FREE_F)(void *);

/**
 * @brief structure of a queue object
 *
 * @param capacity is the number of nodes the queue can hold
 * @param currentsz is the number of nodes the queue is currently storing
 * @param arr is the array containing the queue node pointers
 * @param customfree pointer to the user defined free function
 */
typedef struct queue_p_t
{
    uint32_t capacity;
    uint32_t currentsz;
    queue_p_node_t **arr;
    FREE_F customfree;
} queue_p_t;

/**
 * @brief creates a new queue
 *
 * @param capacity max number of nodes the queue will hold
 * @param customfree pointer to the free function to be used with that list
 *        list
 * @returns pointer to allocated priority queue on success, NULL on failure
 */
queue_p_t *queue_p_init(uint32_t capacity, FREE_F customfree);

/**
 * @brief verifies that queue isn't full
 *
 * @param queue pointer queue object
 * @return 0 on success, non-zero value on failure
 */
int queue_p_fullcheck(queue_p_t *queue);

/**
 * @brief verifies that queue isn't empty
 *
 * @param queue pointer queue object
 * @return 0 on success, non-zero value on failure
 */
int queue_p_emptycheck(queue_p_t *queue);

/**
 * @brief pushes a new node into the queue
 *
 * @param queue pointer to queue pointer to push the node into
 * @param data data to be pushed into node
 * @param priority of data
 * @return 0 on success, non-zero value on failure
 */
int queue_p_enqueue(queue_p_t *queue, void *data, int priority);

/**
 * @brief pops the front node out of the queue
 *
 * @param queue pointer to queue pointer to pop the node off of
 * @return pointer to popped queue node on success, NULL on failure
 */
queue_p_node_t *queue_p_dequeue(queue_p_t *queue);

/**
 * @brief get the data from the node at the front of the queue without popping
 *
 * @param queue pointer to queue pointer to peek
 * @return pointer to popped queue node on success, NULL on failure
 */
queue_p_node_t *queue_p_peek(queue_p_t *queue);

/**
 * @brief clear all nodes out of a queue
 *
 * @param queue pointer to queue pointer to clear out
 * @return 0 on success, non-zero value on failure
 */
int queue_p_clear(queue_p_t *queue);

/**
 * @brief delete a queue
 *
 * @param queue pointer to queue pointer to be destroyed
 * @return 0 on success, non-zero value on failure
 */
int queue_p_destroy(queue_p_t **queue);

/**
 * @brief frees an item and its associated memory
 *
 * @param mem_addr pointer of the item to be free'd
 */
void custom_free(void *mem_addr);
#endif
