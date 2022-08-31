#include "queue_p.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdlib.h>

#define CAPACITY 10

// The queue_p to be used by all the tests
queue_p_t *queue_p = NULL;
// The integer all node data[5] pointers point to
int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int priority_data[5] = {1, 2, 3, 4, 5};

static void printqueue(queue_p_t *queue)
{
    puts("----\n");
    for (int i = 0; i < queue->currentsz; i++)
    {
        printf("%d:%d, ", *(int *)queue->arr[i]->data, queue->arr[i]->priority);
    }
    puts("----\n");
}

static void resetqueue(queue_p_t *queue)
{
    queue_p_node_t *tmp = NULL;
    int i = 0;

    while (queue->currentsz != 0)
    {
        tmp = queue_p_dequeue(queue);
        if (NULL != tmp)
        {
            free(tmp);
            tmp = NULL;
        }
    }
    while (queue->currentsz != CAPACITY)
    {
        queue_p_enqueue(queue, &data[i], 0);
        i++;
    }
}

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_queue_p_init()
{
    uint32_t capacity = CAPACITY;

    // Verify queue_p was created correctly
    queue_p = queue_p_init(capacity, custom_free);
    CU_ASSERT_FATAL(NULL != queue_p);
    // NOLINTNEXTLINE
    CU_ASSERT(CAPACITY == queue_p->capacity);
    // NOLINTNEXTLINE
    CU_ASSERT(0 == queue_p->currentsz);
    // NOLINTNEXTLINE
    CU_ASSERT(NULL != queue_p->arr);
}

void test_queue_p_enqueue()
{
    int exit_code = 1;
    int i = 0;
    queue_p_t *invalid_queue_p = NULL;

    // Should catch if enqueue_p is called on an invalid queue_p or with invalid
    // data
    exit_code = queue_p_enqueue(invalid_queue_p, &data[i], 0);
    CU_ASSERT(0 != exit_code);
    exit_code = queue_p_enqueue(queue_p, NULL, 0);
    CU_ASSERT(0 != exit_code);

    // enqueue_p CAPACITY number of nodes
    while (i < CAPACITY / 2)
    {
        exit_code = queue_p_enqueue(queue_p, &data[i], 0);
        // New node was enqueue_ped and points to the correct data
        CU_ASSERT_FATAL(0 == exit_code);
        CU_ASSERT(data[i] == *(int *)queue_p->arr[i * 2]->data);

        exit_code = queue_p_enqueue(queue_p, &priority_data[i], 1);
        CU_ASSERT_FATAL(0 == exit_code);
        CU_ASSERT(priority_data[i] == *(int *)queue_p->arr[i]->data);
        i++;
    }
    resetqueue(queue_p);

    // Function exited correctly
    CU_ASSERT(0 == exit_code);
    // queue_p size is correct
    CU_ASSERT(CAPACITY == queue_p->currentsz);

    // Function should return a code if enqueue_p is called on a full queue_p
    exit_code = queue_p_enqueue(queue_p, &data[5], 0);
    CU_ASSERT(0 != exit_code);
}

void test_queue_p_dequeue()
{
    int i = 0;
    queue_p_t *invalid_queue_p = NULL;
    queue_p_node_t *node = NULL;

    // Should catch if enqueue_p is called on an invalid queue_p
    node = queue_p_dequeue(invalid_queue_p);
    CU_ASSERT(NULL == node);

    // Dequeue_p all items
    while (queue_p->currentsz > 0)
    {
        node = queue_p_dequeue(queue_p);
        CU_ASSERT_FATAL(NULL != node);
        // NOLINTNEXTLINE
        CU_ASSERT(data[i] == *(int *)node->data);
        free(node);
        node = NULL;
        i++;
    }

    // Should return NULL when called on empty queue_p
    node = queue_p_dequeue(queue_p);
    CU_ASSERT(NULL == node);
}

void test_queue_p_peek()
{
    queue_p_t *invalid_queue_p = NULL;
    queue_p_node_t *node = NULL;
    int i = 0;

    // Should catch if pop is called on an invalid queue_p or empty
    node = queue_p_peek(invalid_queue_p);
    CU_ASSERT(NULL == node);
    node = queue_p_peek(queue_p);
    CU_ASSERT(NULL == node);

    // enqueue_p CAPACITY number of nodes
    while (i < CAPACITY)
    {
        queue_p_enqueue(queue_p, &data[i], 0);
        i++;
    }

    node = queue_p_peek(queue_p);

    // Function should have exited successfully
    CU_ASSERT_FATAL(NULL != node);
    // Correct value should have been peeked from front node
    CU_ASSERT(data[0] == *(int *)(queue_p_node_t *)node->data);
    // Size shouldn't have changed
    CU_ASSERT(CAPACITY == queue_p->currentsz);
}

void test_queue_p_clear()
{
    int exit_code = 1;
    queue_p_t *invalid_queue_p = NULL;

    // Should catch if clear is called on an invalid queue_p
    exit_code = queue_p_clear(invalid_queue_p);
    CU_ASSERT(0 != exit_code);

    exit_code = queue_p_clear(queue_p);
    // queue_p should now be empty
    CU_ASSERT(0 == queue_p->currentsz);
    // Function should have exited fully
    CU_ASSERT(0 == exit_code);
}

void test_queue_p_destroy()
{
    int exit_code = 1;
    queue_p_t *invalid_queue_p = NULL;

    // Should catch if delete is called on an invalid queue_p
    exit_code = queue_p_destroy(&invalid_queue_p);
    CU_ASSERT(0 != exit_code);

    // Function should have exited fully
    exit_code = queue_p_destroy(&queue_p);
    CU_ASSERT(0 == exit_code);

    // Should catch if delte is called on the queue_p that has already been
    // deleted
    exit_code = queue_p_destroy(&queue_p);
    CU_ASSERT(0 != exit_code);
}

int main(void)
{
    CU_TestInfo suite1_tests[] = {
        {"Testing queue_p_init():", test_queue_p_init},

        {"Testing queue_p_enqueue():", test_queue_p_enqueue},

        {"Testing queue_p_dequeue():", test_queue_p_dequeue},

        {"Testing queue_p_peek():", test_queue_p_peek},

        {"Testing queue_p_clear():", test_queue_p_clear},

        {"Testing queue_p_destroy():", test_queue_p_destroy},
        CU_TEST_INFO_NULL};

    CU_SuiteInfo suites[] = {
        {"Suite-1:", init_suite1, clean_suite1, .pTests = suite1_tests},
        CU_SUITE_INFO_NULL};

    if (0 != CU_initialize_registry())
    {
        return CU_get_error();
    }

    if (0 != CU_register_suites(suites))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_basic_show_failures(CU_get_failure_list());
    int num_failed = CU_get_number_of_failures();
    CU_cleanup_registry();
    printf("\n");
    return num_failed;
}
