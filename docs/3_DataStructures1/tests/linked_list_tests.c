#include "linked_list.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>
#include <stdlib.h>

// The list to be used by all the tests
list_t *list = NULL;
// The integer all node data[5] pointers point to
int data[5] = {1, 2, 3, 4, 5};

static void printlist(list_t *list)
{
    int i = 1;
    list_node_t *node = NULL;

    if (NULL != list)
    {
        puts("LIST: ");
        node = list->head;
        while (node != NULL)
        {
            printf("%d", *(int *)node->data);
            if (i == list->size)
            {
                break;
            }
            i++;
            node = node->next;
        }
        puts("\n");
    }
}
static void printlistmembers(list_t *list)
{
    if (NULL != list)
    {
        printf("\nLIST:\t%p,\nSIZE:\t%d\nHEAD:\t%p\nTAIL:\t%p\nFREE:\t%"
               "p\nCOMPARE:\t%p\n",
               list, list->size, list->head, list->tail, list->customfree,
               list->compare_function);
    }
}

void *test_compare_node(int value_to_find, void *node)
{
    // printf("Checking for: %d, found: %d\n", value_to_find, *(int
    // *)((list_node_t *)node)->data);
    void *retval = NULL;

    if (*(int *)((list_node_t *)node)->data == value_to_find)
    {
        // puts("test_compare_node FOUND IT\n");
        retval = node;
    }

    return retval;
}

void make_mod(void *node)
{
    *(int *)((list_node_t *)node)->data =
        ((*(int *)((list_node_t *)node)->data) % 2);
}

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_list_new()
{
    // Verify list was created correctly with no arguments supplied
    list = list_new(NULL, NULL);
    CU_ASSERT_FATAL(NULL != list); // Function exited correctly
    // NOLINTNEXTLINE
    CU_ASSERT(0 == list->size); // list size is correct
    free(list);
    list = NULL;

    // Verify list was created correctly with all arguments supplied
    list = list_new((FREE_F)custom_free, (CMP_F)test_compare_node);
    CU_ASSERT_FATAL(NULL != list); // Function exited correctly
    // NOLINTNEXTLINE
    CU_ASSERT(0 == list->size); // list size is correct
}

void test_list_push_tail()
{
    int exit_code = 1;
    int i = 0;
    list_t *invalid_list = NULL;

    // Should catch if push is called on an invalid list
    CU_ASSERT_FATAL(NULL != list);
    CU_ASSERT(0 != list_push_tail(invalid_list, &data[0]));

    while (i < 5)
    {
        exit_code = list_push_tail(list, &data[i]);
        if (0 != exit_code)
        {
            break;
        }
        // New node was pushed and points to the correct data
        CU_ASSERT(data[i] == *(int *)(list->tail->data));
        i++;
    }

    // Function exited correctly
    CU_ASSERT(0 == exit_code);
    // list size is correct
    // NOLINTNEXTLINE
    CU_ASSERT(5 == list->size);
}

void test_list_is_circular()
{
    CU_ASSERT_FATAL(NULL != list);
    // NOLINTNEXTLINE
    CU_ASSERT(list->tail->next == list->head);
}

void test_list_pop_head()
{
    int exit_code = 1;
    int *value_from_node = NULL;
    int i = 0;
    list_t *invalid_list = NULL;
    list_node_t *node = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if pop is called on an invalid list
    CU_ASSERT(NULL == list_pop_head(invalid_list));

    // Pop all nodes
    while (i < 5)
    {
        node = list_pop_head(list);
        CU_ASSERT_FATAL(NULL != node);
        // NOLINTNEXTLINE
        CU_ASSERT(data[i] == *(int *)node->data);
        free(node);

        i++;
    }
    // NOLINTNEXTLINE
    CU_ASSERT(0 == list->size);
    // Function should return NULL if pop is called on an empty list
    CU_ASSERT(NULL == list_pop_head(list));
}

void test_list_push_head()
{
    int exit_code = 1;
    int i = 0;
    list_t *invalid_list = NULL;

    // Should catch if push is called on an invalid list
    CU_ASSERT_FATAL(NULL != list);
    exit_code = list_push_head(invalid_list, &data[i]);
    CU_ASSERT(0 != exit_code);

    // Push 5 nodes
    while (i < 5)
    {
        exit_code = list_push_head(list, &data[i]);
        // New node was pushed and points to the correct data
        // NOLINTNEXTLINE
        CU_ASSERT(data[i] == *(int *)(list->head->data));

        i++;
    }

    // Function exited correctly
    CU_ASSERT(0 == exit_code);
    // list size is correct
    // NOLINTNEXTLINE
    CU_ASSERT(5 == list->size);
}

void test_list_sort()
{
    int exit_code = 1;
    list_t *invalid_list = NULL;
    list_node_t *node = NULL;
    int i = 0;

    // Should catch if remove is called on an invalid list
    exit_code = list_sort(invalid_list);
    CU_ASSERT(0 != exit_code);

    exit_code = list_sort(list);
    // Ensure function exited successfully
    CU_ASSERT(0 == exit_code);

    // Verify list should now be reversed
    while (i < 5)
    {
        node = list_pop_head(list);
        CU_ASSERT_FATAL(NULL != node);
        // Correct value should be successfully popped from head
        // NOLINTNEXTLINE
        CU_ASSERT(data[i] == *(int *)node->data);
        free(node);
        i++;
    }

    // Return the list to its original state for following tests
    i = 0;
    while (i < 5)
    {
        list_push_head(list, &data[i]);
        i++;
    }
}

void test_list_pop_tail()
{
    int exit_code = 0;
    int i = 0;
    list_t *invalid_list = NULL;
    list_node_t *node = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if pop is called on an invalid list
    node = list_pop_tail(invalid_list);
    CU_ASSERT(NULL == node);

    // Pop all nodes
    while (i < 5)
    {
        node = list_pop_tail(list);
        // Correct value should be successfully popped into value_from_node
        CU_ASSERT_FATAL(NULL != node);
        // NOLINTNEXTLINE
        CU_ASSERT(data[i] == *(int *)node->data);
        free(node);
        node = NULL;

        i++;
    }

    CU_ASSERT(0 == list->size);

    // Function should return null if pop is called on an empty list
    node = list_pop_head(list);
    CU_ASSERT(NULL == node);
}

void test_list_peek_head()
{
    list_t *invalid_list = NULL;
    list_node_t *node = NULL;
    int i = 0;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if function is called on an invalid list
    node = list_peek_head(invalid_list);
    CU_ASSERT(NULL == node);

    // Function should not be able to peek on an empty list
    node = list_peek_head(list);
    CU_ASSERT(NULL == node);

    // Push the nodes tail into list
    while (i < 5)
    {
        list_push_tail(list, &data[i]);
        i++;
    }

    node = list_peek_head(list);

    // Function should have exited successfully
    CU_ASSERT_FATAL(NULL != node);
    // Correct value should have been peeked from head node
    CU_ASSERT(data[0] == *(int *)node->data);
    // Size shouldn't have changed
    // NOLINTNEXTLINE
    CU_ASSERT(5 == list->size);
}

void test_list_remove()
{
    int exit_code = 1;
    int value_to_remove = 4;
    list_t *invalid_list = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if remove is called on an invalid list
    exit_code = list_remove(invalid_list, (void *)&value_to_remove);
    CU_ASSERT(0 != exit_code);

    exit_code = list_remove(list, (void *)&value_to_remove);
    // Function should have exited successfully
    CU_ASSERT(0 == exit_code);
    // Size should reflect the removal of the node
    // NOLINTNEXTLINE
    CU_ASSERT(4 == list->size);

    // The node containing the removed value should no longer be in the list
    exit_code = list_remove(list, (void *)&value_to_remove);
    CU_ASSERT(0 != exit_code);
}

void test_list_find_first_occurrence()
{
    int value_to_find = 4;
    list_t *invalid_list = NULL;
    list_node_t *node = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if function is called on an invalid list
    node = list_find_first_occurrence(invalid_list, (void *)&value_to_find);
    CU_ASSERT(NULL == node);

    // Should not be able to find the value removed in the previous test
    node = list_find_first_occurrence(list, (void *)&value_to_find);
    CU_ASSERT(NULL == node);

    // Change the value we are looking for to one that is still in the list
    value_to_find--;
    node = list_find_first_occurrence(list, (void *)&value_to_find);

    // Ensure function exited successfully
    CU_ASSERT_FATAL(NULL != node);
    // Ensure the value found was the one that was searched for
    // NOLINTNEXTLINE
    CU_ASSERT(value_to_find == *(int *)node->data);
}

void test_list_foreach_call()
{
    int exit_code = 1;
    list_t *invalid_list = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if function is called on an invalid list
    exit_code = list_foreach_call(invalid_list, make_mod);
    CU_ASSERT(0 != exit_code);

    exit_code = list_foreach_call(list, make_mod);

    // Ensure function exited correctly
    CU_ASSERT(0 == exit_code);

    // Ensure the user defined action was done to the list nodes
    // NOLINTNEXTLINE
    CU_ASSERT(1 == *(int *)list->head->data &&
              0 == *(int *)list->head->next->data);
}

void test_list_find_all_occurrences()
{
    int value_to_find = 1;
    list_t *test_list = NULL;
    list_t *result_list = NULL;
    list_node_t *tmp = NULL;
    unsigned int i = 0;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if function is called on an invalid list
    result_list = list_find_all_occurrences(test_list, (void *)&value_to_find);
    CU_ASSERT(NULL == result_list);

    // create empty list
    test_list = list_new((FREE_F)custom_free, (CMP_F)test_compare_node);

    // Should catch if function is called on an empty list
    result_list = list_find_all_occurrences(test_list, (void *)&value_to_find);
    CU_ASSERT(NULL == result_list);

    result_list = list_find_all_occurrences(list, (void *)&value_to_find);

    // Ensure function exited successfully
    CU_ASSERT_FATAL(NULL != result_list);
    // Verify that each node of the resulting list is the correct value
    // NOLINTNEXTLINE
    tmp = result_list->head;
    while (i < result_list->size)
    {
        CU_ASSERT(value_to_find == *(int *)tmp->data);
        tmp = tmp->next;
        i++;
    }

    // Verify that the correct number of occurrences were found
    // NOLINTNEXTLINE
    CU_ASSERT(3 == result_list->size);

    list_delete(&test_list);
    list_delete(&result_list);
}

void test_list_clear()
{
    int exit_code = 1;
    list_t *invalid_list = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if clear is called on an invalid list
    exit_code = list_clear(invalid_list);
    CU_ASSERT(0 != exit_code);

    exit_code = list_clear(list);
    // list should now be empty
    // NOLINTNEXTLINE
    CU_ASSERT(0 == list->size)
    // NOLINTNEXTLINE
    CU_ASSERT(NULL == list->head)

    // Function should have exited successfully
    CU_ASSERT(0 == exit_code);
}

void test_list_peek_tail()
{
    list_t *invalid_list = NULL;
    list_node_t *node = NULL;
    int i = 0;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if pop is called on an invalid list
    node = list_peek_tail(invalid_list);
    CU_ASSERT(NULL == node);

    // Function should not be able to peek on an empty list
    node = list_peek_tail(list);
    CU_ASSERT(NULL == node);

    // Push the nodes tail into list
    while (i < 5)
    {
        list_push_tail(list, &data[i]);
        i++;
    }

    node = list_peek_tail(list);

    // Function should have exited successfully
    CU_ASSERT_FATAL(NULL != node);
    // Correct value should have been peeked from head node
    // NOLINTNEXTLINE
    CU_ASSERT(data[4] == *(int *)node->data);
    // Size shouldn't have changed
    CU_ASSERT(5 == list->size);
}

void test_list_delete()
{
    int exit_code = 1;
    list_t *invalid_list = NULL;

    CU_ASSERT_FATAL(NULL != list);
    // Should catch if delete is called on an invalid list
    exit_code = list_delete(&invalid_list);
    CU_ASSERT(0 != exit_code);

    // Function should have exited successfully
    exit_code = list_delete(&list);
    CU_ASSERT(0 == exit_code);

    // Should catch if delete is called on the list that has already been
    // deleted
    exit_code = list_delete(&list);
    CU_ASSERT(0 != exit_code);
}

int main(void)
{
    CU_TestInfo suite1_tests[] = {
        {"Testing list_new():", test_list_new},

        {"Testing list_push_tail():", test_list_push_tail},

        {"Testing if the resulting list is circular:", test_list_is_circular},

        {"Testing list_pop_head():", test_list_pop_head},

        {"Testing list_push_head():", test_list_push_head},

        {"Testing if the resulting list is still circular:",
         test_list_is_circular},

        {"Testing list_sort():", test_list_sort},

        {"Testing list_pop_tail():", test_list_pop_tail},

        {"Testing list_peek_head():", test_list_peek_head},

        {"Testing list_remove():", test_list_remove},

        {"Testing list_find_first_occurrence():",
         test_list_find_first_occurrence},

        {"Testing list_find_first_foreach_call():", test_list_foreach_call},

        {"Testing list_find_all_occurrences():",
         test_list_find_all_occurrences},

        {"Testing list_clear():", test_list_clear},

        {"Testing list_peek_tail():", test_list_peek_tail},

        {"Testing list_delete():", test_list_delete},
        CU_TEST_INFO_NULL};

    CU_SuiteInfo suites[] = {
        {"Suite-1:", init_suite1, clean_suite1, .pTests = suite1_tests},
        CU_SUITE_INFO_NULL};

    if (CUE_SUCCESS != CU_initialize_registry())
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
