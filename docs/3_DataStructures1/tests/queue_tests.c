#include "queue.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdlib.h>

#define CAPACITY 5

// The queue to be used by all the tests
queue_t *queue = NULL;
// The integer all node data[5] pointers point to
int data[5] = {1, 2, 3, 4, 5};

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_queue_init()
{
    uint32_t capacity = CAPACITY;

    // Verify queue was created correctly
    queue = queue_init(capacity, custom_free);
    CU_ASSERT_FATAL(NULL != queue);
    // NOLINTNEXTLINE
    CU_ASSERT(CAPACITY == queue->capacity);
    // NOLINTNEXTLINE
    CU_ASSERT(0 == queue->currentsz);
    // NOLINTNEXTLINE
    CU_ASSERT(NULL != queue->arr);
}

void test_queue_enqueue()
{
    int exit_code = 1;
    int i = 0;
    queue_t *invalid_queue = NULL;

    // Should catch if enqueue is called on an invalid queue or with invalid
    // data
    exit_code = queue_enqueue(invalid_queue, &data[i]);
    CU_ASSERT(0 != exit_code);
    exit_code = queue_enqueue(queue, NULL);
    CU_ASSERT(0 != exit_code);

    // enqueue CAPACITY number of nodes
    while (i < CAPACITY)
    {
        exit_code = queue_enqueue(queue, &data[i]);
        // New node was enqueueed and points to the correct data
        CU_ASSERT(data[i] == (*(int *)queue->arr[i]->data));
        i++;
    }

    // Function exited correctly
    CU_ASSERT(0 == exit_code);
    // queue size is correct
    CU_ASSERT(CAPACITY == queue->currentsz);

    // Function should return a code if enqueue is called on a full queue
    exit_code = queue_enqueue(queue, &data[5]);
    CU_ASSERT(0 != exit_code);
}

void test_queue_dequeue()
{
    int i = 0;
    queue_t *invalid_queue = NULL;
    queue_node_t *node = NULL;

    // Should catch if enqueue is called on an invalid queue
    node = queue_dequeue(invalid_queue);
    CU_ASSERT(NULL == node);

    // Dequeue all items
    while (queue->currentsz > 0)
    {
        node = queue_dequeue(queue);
        CU_ASSERT_FATAL(NULL != node);
        // NOLINTNEXTLINE
        CU_ASSERT(data[i] == *(int *)node->data);
        free(node);
        node = NULL;
        i++;
    }

    // test adding less than the queue's total capacity before removing items
    queue_enqueue(queue, &data[0]);
    queue_enqueue(queue, &data[1]);

    node = queue_dequeue(queue);
    CU_ASSERT(data[0] == *(int *)node->data);
    free(node);

    node = queue_dequeue(queue);
    CU_ASSERT(data[1] == *(int *)node->data);
    free(node);

    // test adding and removing a single item, twice
    queue_enqueue(queue, &data[0]);
    node = queue_dequeue(queue);
    CU_ASSERT(data[0] == *(int *)node->data);
    free(node);

    queue_enqueue(queue, &data[1]);
    node = queue_dequeue(queue);
    CU_ASSERT(data[1] == *(int *)node->data);
    free(node);

    // Should return NULL when called on empty queue
    node = queue_dequeue(queue);
    CU_ASSERT(NULL == node);
}

void test_queue_peek()
{
    queue_t *invalid_queue = NULL;
    queue_node_t *node = NULL;
    int i = 0;

    // Should catch if pop is called on an invalid queue or empty
    node = queue_peek(invalid_queue);
    CU_ASSERT(NULL == node);
    node = queue_peek(queue);
    CU_ASSERT(NULL == node);

    // enqueue CAPACITY number of nodes
    while (i < CAPACITY)
    {
        queue_enqueue(queue, &data[i]);
        i++;
    }

    node = queue_peek(queue);

    // Function should have exited successfully
    CU_ASSERT_FATAL(NULL != node);
    // Correct value should have been peeked from front node
    CU_ASSERT(data[0] == *(int *)node->data);
    // Size shouldn't have changed
    CU_ASSERT(CAPACITY == queue->currentsz);
}

void test_queue_clear()
{
    int exit_code = 1;
    queue_t *invalid_queue = NULL;

    // Should catch if clear is called on an invalid queue
    exit_code = queue_clear(invalid_queue);
    CU_ASSERT(0 != exit_code);

    exit_code = queue_clear(queue);
    // queue should now be empty
    CU_ASSERT(0 == queue->currentsz);
    // Function should have exited fully
    CU_ASSERT(0 == exit_code);
}

void test_queue_destroy()
{
    int exit_code = 1;
    queue_t *invalid_queue = NULL;

    // Should catch if delete is called on an invalid queue
    exit_code = queue_destroy(&invalid_queue);
    CU_ASSERT(0 != exit_code);

    // Function should have exited fully
    exit_code = queue_destroy(&queue);
    CU_ASSERT(0 == exit_code);

    // Should catch if delte is called on the queue that has already been
    // deleted
    exit_code = queue_destroy(&queue);
    CU_ASSERT(0 != exit_code);
}

int main(void)
{
    CU_TestInfo suite1_tests[] = {
        {"Testing queue_init():", test_queue_init},

        {"Testing queue_enqueue():", test_queue_enqueue},

        {"Testing queue_dequeue():", test_queue_dequeue},

        {"Testing queue_peek():", test_queue_peek},

        {"Testing queue_clear():", test_queue_clear},

        {"Testing queue_destroy():", test_queue_destroy}, CU_TEST_INFO_NULL};

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
