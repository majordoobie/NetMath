#include <gtest/gtest.h>
#include <header_parser.h>
#include <fcntl.h>


int possible_paths = 3;
const char * base_paths[3] = {"tests/equ_files/",
                              "../../tests/equ_files/",
                              "../../../../tests/equ_files/"};

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
    EXPECT_EQ(magic_filed, MAGIC_VALUE);
    close(file);
}
