
#!/bin/python3

from subprocess import Popen, PIPE
import os

FAIL = -1
PASS = 0

bin_loc = "3_DataStructures1/build/"
tests = ['test_list', 'test_queue', 'test_queue_p', 'test_stack', 'test_table']

def test_binary(binary):
    p = Popen([binary], stdin=PIPE, stdout=PIPE, stderr=PIPE, cwd=bin_loc)
    output, err = p.communicate()
    str_out = output.decode("ascii")

    if p.returncode == 0:
        return PASS
    else:
        print(f"{binary} Failed!")
        print("Failing Output")
        print(output.decode("ascii"))
        print(err.decode("ascii"))
        return FAIL

def test_valgrind(binary):
    p = Popen(['valgrind', '--leak-check=full', '--error-exitcode=1', binary], stdin=PIPE, stdout=PIPE, stderr=PIPE, cwd=bin_loc)
    output, err = p.communicate()
    str_out = output.decode("ascii")

    if p.returncode == 0:
        return PASS
    else:
        print(f"{binary} Failed!")
        print("Failing Output")
        print(output.decode("ascii"))
        print(err.decode("ascii"))
        return FAIL

def setup():
    os.chdir("3_DataStructures1/")
    os.system("./build.sh")
    os.chdir("..")

def main():
    setup()
    numtests = 0
    numpassed = 0

    for test in tests:
        numtests += 1
        res1 = test_binary(f"./{test}")
        res2 = test_valgrind(f"./{test}")
        numpassed = numpassed + 1 if (PASS == res1 and PASS == res2) else numpassed


    print("\nPassed {} out of {} ".format(numpassed, numtests))

    ret = 0 if numtests == numpassed else -1

    exit(ret)


if __name__ == "__main__":
    main()