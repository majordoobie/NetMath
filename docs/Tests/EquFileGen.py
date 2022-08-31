import argparse
import random
import struct
import os

# Define Sizes used for various elements
EQU_HDR_SIZE = 27
SER_EQU_SIZE = 32
EQU_SIZE = 17
SOLV_EQU_SIZE = 12

# Magic values
EQU_HDR_MAGIC = 0xdd77bb55

# Assign Operators to Values
operators = {
    'add'  :b'\x01',
    'sub'  :b'\x02',
    'mult' :b'\x03',
    'div'  :b'\x04',
    'mod'  :b'\x05',
    # 'ls'   :b'\x06',
    # 'rs'   :b'\x07',
    'and'  :b'\x08',
    'or'   :b'\x09',
    'xor'  :b'\x0a',
    # 'rol'  :b'\x0b',
    # 'ror'  :b'\x0c'
}

file_hdr_flags = {
    "solved"   :b'\x01',
    "unsolved" :b'\x00'
}


#Generates the Equation header and file based on parameters
class EquFile():

    def set_flag(self, flag):
        self.flags = ord(flag) | ord(self.flags)
        self.flags = self.flags.to_bytes(1, "little")


    def __init__(self):
        self.magic = EQU_HDR_MAGIC
        self.file_id = os.urandom(8)

        self.flags = b'\x00'
        self.set_flag(file_hdr_flags["unsolved"])

        #Equations start right after EquHeader
        self.equ_offset = EQU_HDR_SIZE 

        # File Calc Does not impliment opt headers
        self.num_opt_hdrs = 0

        self.num_equ = 0
        self.equations = []


    def gen_equations(self, num_equ):
        self.num_equ = num_equ

        for x in range(self.num_equ):
            # Generate Equation Header
            equ_id = os.urandom(4)
            flags = b"\x00"
            padding = b"\x00"*10           

            # Generate Equation
            op = random.choice(list(operators.values()))
            if int.from_bytes(op, "little") <= 0x05:
                num1 = struct.pack("<Q", random.randint(0, 2 ** 16))
                num2 = struct.pack("<Q", random.randint(0, 2 ** 16))
            else:
                num1 = struct.pack("<Q", random.randint(0, 2 ** 16))
                num2 = struct.pack("<Q", random.randint(0, 16))


            equ = equ_id + flags + (num1 + op + num2) + padding
            self.equations.append(equ)


    def write_file(self, filename):
        with open(filename, "wb") as eqf:
            eqf.write(struct.pack("I", self.magic)) 
            eqf.write(self.file_id)
            eqf.write(struct.pack("<Q", self.num_equ))
            eqf.write(struct.pack("<c", self.flags))
            eqf.write(struct.pack("<I", self.equ_offset))
            eqf.write(struct.pack("<H", self.num_opt_hdrs))

            for e in self.equations:
                eqf.write(e)
            
def main():

    output_dir = "."
    num_files = 0
    num_equ = 0

    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output-dir", help="output directory for all the files. note default is current dir and will replace all .equ files")
    parser.add_argument("-nf", "--num-files", help="number of files to generate. max is 2048. default is 16")
    parser.add_argument("-ne", "--num-equ", help="number of equations per file. max is 4096. default is 64")
    args = parser.parse_args()

    if args.output_dir:
        try:
            os.mkdir(args.output_dir)
            output_dir = args.output_dir

        except:
            output_dir = args.output_dir

    if not args.num_files:
        num_files = 16
    else:
        num_files = int(args.num_files)

    if not args.num_equ:
        num_equ = 64
    else:
        num_equ = int(args.num_equ)

    if num_equ > 4096 or num_files > 2048:
        print("too many")
        exit(-1)

    print(f"Generating {num_files} files with {num_equ} equations")

    for f in range(num_files):
        equ_file = EquFile()
        equ_file.gen_equations(num_equ)
        equ_file.write_file(f"{output_dir}/{equ_file.file_id.hex()}.equ")


if __name__ == "__main__":
    main()