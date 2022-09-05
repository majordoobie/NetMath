#include <gtest/gtest.h>
#include <header_parser.h>
#include <fcntl.h>


int possible_paths = 3;
// The path can change depending on how you are running the tests. If running
// from builder.py then the second path is used. If running from the test
// bind folder then the last path is used.
const char * base_paths[3] = {"tests/unsolved/",
                              "../../tests/unsolved/",
                              "../../../../tests/unsolved/"};

int get_file(const char * file_name)
{
    for (int i = 0; i < possible_paths; i++)
    {
        // Plus one for the null
        char full_path[NAME_MAX];
        snprintf(full_path, NAME_MAX, "%s%s", base_paths[i], file_name);

        int file = open(full_path, O_CLOEXEC | O_RDONLY);
        if (-1 != file)
        {
            return file;
        }
    }
    return -1;
}

TEST(Simple, Simple)
{
    const char * file_name = "valid_3a78786203e77c0b.equ";
    int file = get_file(file_name);
    ASSERT_NE(file, -1) << "[!] File provided " << file_name << " was not found\n";

    uint32_t magic_filed = 0;
    read(file, &magic_filed, HEAD_MAGIC);
    ASSERT_EQ(magic_filed, MAGIC_VALUE);
    lseek(file, 0, SEEK_SET);

    equations_t * eqs = parse_stream(file);
    ASSERT_NE(eqs, nullptr);

    // Test th of the opts are within the accepted range. Any miss alignment
    // should show up here
    unsolved_eq_t * un_eq = eqs->eqs;
    while (NULL != un_eq)
    {
        EXPECT_TRUE((un_eq->opt >= 0) && (un_eq->opt <= 0x0c));
        un_eq = un_eq->next;
    }

    free_equation(eqs);
    close(file);
}


