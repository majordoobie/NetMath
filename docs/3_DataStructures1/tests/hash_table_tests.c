#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <hash_table.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
hash_table_t *hash_table = NULL;
int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int properly_implemented_free = 1;

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_hash_table_init()
{
    int exit_code = 1;

    // Verify hash_table was created correctly
    hash_table = hash_table_init(SIZE, NULL);
    CU_ASSERT_FATAL(NULL != hash_table);
    CU_ASSERT(NULL != hash_table->table); // NOLINT
    CU_ASSERT(SIZE == hash_table->size);
    // Ensure that free is substituted in the event that a custom
    // free function isnt supplied
    CU_ASSERT(free == hash_table->customfree);
}
//////////////////////////////////
// TODO: FINISH REFACTORING BELOW//
//////////////////////////////////

void test_hash_table_add()
{
    hash_table_t *invalid_table = NULL;

    // Should catch if create is called on an invalid pointer
    CU_ASSERT(FAILURE ==
              hash_table_add(invalid_table, (void *)&data[3], "asdgasga"));

    // Should all work
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[0], "Item one"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[1], "Item two"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[2], "Item three"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[3], "Item four"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[4], "Item five"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[5], "Item six"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[6], "Item seven"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[7], "Item eight"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[8], "Item nine"));
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[9], "Item ten"));
}

void test_hash_table_lookup()
{
    void *return_ptr = NULL;
    char test_key[10] = {0};
    hash_table_t *invalid_table = NULL;

    // Should catch if create is called on an invalid pointer
    return_ptr = hash_table_lookup(invalid_table, "Item three");
    CU_ASSERT(NULL == return_ptr);

    // Check for ptr comparision of key value
    strncpy(test_key, "key1", 10);
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[0], test_key));
    strncpy(test_key, "key2", 10);
    CU_ASSERT(SUCCESS ==
              hash_table_add(hash_table, (void *)&data[0], test_key));
    return_ptr = hash_table_lookup(hash_table, test_key);
    CU_ASSERT((void *)&test_key != return_ptr);

    // ensure unique nodes are created per key value
    // each node is created with data fields of the same value
    return_ptr = hash_table_lookup(hash_table, "key1");
    CU_ASSERT_FATAL(NULL != return_ptr);
    CU_ASSERT_FATAL(return_ptr == hash_table_lookup(hash_table, "key2"));

    // check normal returns
    return_ptr = hash_table_lookup(hash_table, "Item two");
    CU_ASSERT_FATAL((void *)&data[1] == return_ptr);

    return_ptr = hash_table_lookup(hash_table, "Item three");
    CU_ASSERT_FATAL((void *)&data[2] == return_ptr);
}

void test_hash_table_remove()
{
    int exit_code = 1;
    void *ret_ptr = NULL;
    hash_table_t *invalid_table = NULL;

    exit_code = hash_table_remove(invalid_table, "Item three");
    CU_ASSERT(FAILURE == exit_code);

    ret_ptr = hash_table_lookup(hash_table, "Item three");
    CU_ASSERT(NULL != ret_ptr);

    exit_code = hash_table_remove(hash_table, "Item three");
    CU_ASSERT(SUCCESS == exit_code);

    ret_ptr = hash_table_lookup(hash_table, "Item three");
    CU_ASSERT_FATAL(NULL == ret_ptr);

    exit_code = hash_table_remove(hash_table, "Item three");
    CU_ASSERT(FAILURE == exit_code);
}

void test_hash_table_clear()
{
    int exit_code = 1;
    hash_table_t *invalid_table = NULL;

    exit_code = hash_table_clear(invalid_table);
    CU_ASSERT(FAILURE == exit_code);

    exit_code = hash_table_clear(hash_table);
    CU_ASSERT(SUCCESS == exit_code);
}

void test_hash_table_destroy()
{
    int exit_code = 1;
    hash_table_t *invalid_table = NULL;

    exit_code = hash_table_destroy(&invalid_table);
    CU_ASSERT(FAILURE == exit_code);

    exit_code = hash_table_destroy(&hash_table);
    CU_ASSERT(SUCCESS == exit_code);

    exit_code = hash_table_destroy(&hash_table);
    CU_ASSERT(FAILURE == exit_code);
}

int main(void)
{
    CU_TestInfo suite1_tests[] = {
        {"Testing hash_table_init():", test_hash_table_init},

        {"Testing hash_table_add():", test_hash_table_add},

        {"Testing hash_table_lookup():", test_hash_table_lookup},

        {"Testing hash_table_remove():", test_hash_table_remove},

        {"Testing hash_table_clear():", test_hash_table_clear},

        {"Testing hash_table_destroy():", test_hash_table_destroy},

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
    puts("\n");
    return num_failed;
}
