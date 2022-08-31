import os
import EquGrader

            
def run_binary():
    os.system("./build/2_FileCalc/filecalc ./filecalc_tests/unsolved ./filecalc_tests/solved")


def main():
    result = 0
    tests_base =  "./filecalc_tests/"
    num_files = 16
    num_equ = 64

    EquGrader.setup(tests_base, num_files, num_equ)
    run_binary()
    result = EquGrader.grade_dirs(tests_base)
    EquGrader.cleanup(tests_base)

    return result



if __name__ == "__main__":
    main()