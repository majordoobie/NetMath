#include <gtest/gtest.h>
#include <header_parser.h>
#include <string.h>


int possible_paths = 3;
const char * base_paths[3] = {"tests/equ_files/",
                              "../../tests/equ_files/",
                              "../../../../tests/equ_files/"};

FILE * get_file(const char * file_name)
{
    for (int i = 0; i < possible_paths; i++)
    {
        // Plus one for the null
        char full_path[NAME_MAX];
        snprintf(full_path, NAME_MAX, "%s%s", base_paths[i], file_name);

        FILE * file = fopen(full_path, "rb");
        if (NULL != file)
        {
            return file;
        }
    }
    fprintf(stderr, "Unable to find file %s\n", file_name);
    return NULL;
}

TEST(Simple, Simple)
{
    FILE * file = get_file("3a78786203e77c0b.equ");
    EXPECT_NE(file, nullptr);
    fclose(file);
}
