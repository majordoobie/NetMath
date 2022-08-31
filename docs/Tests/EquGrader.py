import os
import struct
import random
import os
import shutil

# Define Sizes used for various elements
EQU_HDR_SIZE = 27
SER_EQU_SIZE = 32
EQU_SIZE = 17
SOLV_EQU_SIZE = 14

# Magic values
EQU_HDR_MAGIC = 0xdd77bb55

# Assign Operators to Values
OPERATORS = {
    "ADD":      b"\x01",
    "SUBTRACT": b"\x02",
    "MULT":     b"\x03",
    "DIVIDE":   b"\x04",
    "MOD":      b"\x05",
    "LSHIFT":   b"\x06",
    "RSHIFT":   b"\x07",
    "AND":      b"\x08",
    "OR":       b"\x09",
    "XOR":      b"\x0A",
    "LROT":     b"\x0B",
    "RROT":     b"\x0C"
}

file_hdr_flags = {
    "solved"   :b'\x01',
    "unsolved" :b'\x00'
}

def generate_files(num_files, num_equ, output_dir):

    print(f"Generating {num_files} files with {num_equ} equations")

    for f in range(num_files):
        equ_file = EquFile()
        equ_file.gen_equations(num_equ)
        equ_file.write_file(f"{output_dir}/{equ_file.file_id.hex()}.equ")

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

        # File Calc Does not implement opt headers
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
            op = random.choice(list(OPERATORS.values()))
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

class UnpackedHeader():

    def __init__(self, infilebuf):
        self.flags = {
            "UNSOLVED": b"\0x00",
            "SOLVED":   b"\0x01"
        }
        self.magic = struct.unpack_from("<I", infilebuf, offset=0)[0]
        self.fileid = struct.unpack_from("<Q", infilebuf, offset=4)[0]
        self.numequ = struct.unpack_from("<Q", infilebuf, offset=12)[0]
        self.flag = infilebuf[20]
        self.offset = struct.unpack_from("<I", infilebuf, offset=21)[0]
        self.numopt = struct.unpack_from("<H", infilebuf, offset = 25)[0]

    def __str__(self):
        return "--HEADER--\nMagic:\t\t{}\nID:\t\t{}\nNum Equ:\t{}\nFlag:\t\t{}\nOffset:\t\t{}\nOpt Hdrs:\t{}".format(self.magic, self.fileid, self.numequ, self.flag, self.offset, self.numopt)


class Equation():

    def __init__(self, bytes):
        self.operations = OPERATORS
        self.opkeys = list(self.operations.keys())
        self.opval = list(self.operations.values())

        self.flag = struct.unpack_from("<c", bytes, offset=4)[0]
        self.operator = struct.unpack_from("<c", bytes, offset=13)[0]
        self.op_pos = self.opval.index(self.operator)


        if int.from_bytes(self.operator, "little") > int.from_bytes(self.operations["MOD"], "little"):
            """Treat as unsigned"""
            self.id = struct.unpack_from("<I", bytes, offset=0)[0]
            self.op1 = struct.unpack_from("<Q", bytes, offset=5)[0]
            self.op2 = struct.unpack_from("<Q", bytes, offset=14)[0]
            self.padding = bytes[25:32]
            self.data_code = 0x02;
        else:
            """Treat as signed"""
            self.id = struct.unpack_from("<I", bytes, offset=0)[0]
            self.op1 = struct.unpack_from("<q", bytes, offset=5)[0]
            self.op2 = struct.unpack_from("<q", bytes, offset=14)[0]
            self.padding = bytes[25:32]
            self.data_code = 0x01;


    def __str__(self):
        try:
            return "\t\t--Equation--\n\tID:\t\t{}\n\tOperand 1:\t{}\n\tOperator:\t{}\n\tOperand 2:\t{}".format(self.id, self.op1, self.opkeys[self.op_pos], self.op2)
        except KeyError:
            return "\t\t--Equation--\n\tID:\t\t{}\n\tOperand 1:\t{}\n\tOperator:\t{}\n\tOperand 2:\t{}".format(self.id, self.op1, self.operator, self.op2)

class Solution():

    def __init__(self, equation: Equation):
        self.operations = {
            b"\x01": self.add,
            b"\x02": self.subtract,
            b"\x03": self.multiply,
            b"\x04": self.divide,
            b"\x05": self.mod,
            b"\x06": self.bitlshift,
            b"\x07": self.bitrshift,
            b"\x08": self.bitand,
            b"\x09": self.bitor,
            b"\x0a": self.bitxor,
            b"\x0b": self.bitlrot,
            b"\x0c": self.bitrrot
        }

        self.eqflags = {
            "EQ_SOLVED":    0x1,
            "ERROR":        0x0
        }

        self.dtflags = {
            "SIGNED": 0x01,
            "UNSIGED": 0x02,
            "FLOAT": 0x03
        }


        if isinstance(equation, bytes):
            self.id = struct.unpack_from("<I", equation, offset=0)[0]
            self.eqflag = equation[4]
            self.data_code = equation[5]
            if (self.data_code == 1):
                self.solution = struct.unpack_from("<q", equation, offset=6)[0]
            else:
                self.solution = struct.unpack_from("<Q", equation, offset=6)[0]

        else:
            self.long_max = (2 ** 63)
            self.long_min = (2 ** 64)
            self.equation = equation
            self.id = equation.id
            self.data_code = equation.data_code
            self.eqflag = 0
            self.solution = 0
            try:
                self.operations[self.equation.operator]()
                self.eqflag = self.eqflags["EQ_SOLVED"]
            except Exception as e:
                print(e)
        #TODO: Support bitwise shifting and rotating
        # if isinstance(self.eqflag, int):
        #     if self.eqflag == 0:
        #         self.byteflag = b"\x00"
        #     else:
        #         self.byteflag = b"\x01"
        # else:
        #     self.byteflag = self.eqflag

        # self.rawbytes = struct.pack("<IcQ", self.id, self.byteflag, self.solution)
    

    def __str__(self):
        return "\t\t--Solution--\n\tID: {}, Flag: {}, DataCode: {}, Solution: {}".format(self.id, self.eqflag, self.data_code, self.solution)

    def __bytes__(self):
        bid = self.id.to_bytes(4, 'little')
        bflags = self.eqflag.to_bytes(1, 'little')
        bdatacode = self.data_code.to_bytes(1, 'little')
        bsolv = struct.pack("Q", self.solution)
        return bid + bflags + bdatacode + bsolv

    def add(self):
            
        self.solution = self.equation.op1 + self.equation.op2
        if self.long_min > self.solution > self.long_max:
            self.eqflag = self.eqflags["ERROR"]

    def subtract(self):

        self.solution = self.equation.op1 - self.equation.op2
        if self.long_min > self.solution > self.long_max:
            self.eqflag = self.eqflags["ERROR"]


    def multiply(self):

        self.solution = self.equation.op1 * self.equation.op2
        if self.long_min > self.solution > self.long_max:
            self.eqflag = self.eqflags["ERROR"]

    def divide(self):

        try:
            self.solution = self.equation.op1 // self.equation.op2
        except ZeroDivisionError:
            self.eqflag = self.eqflags["ERROR"]
        else:
            if self.long_min > self.solution > self.long_max:
                self.eqflag = self.eqflags["ERROR"]

    def mod(self):

        try:
            self.solution = self.equation.op1 % self.equation.op2
        except ZeroDivisionError:
            self.eqflag = self.eqflags["ERROR"]
        else:
            if self.long_min > self.solution > self.long_max:
                self.eqflag = self.eqflags["ERROR"]

    def bitlshift(self):

        try:
            self.solution = self.equation.op1 << self.equation.op2
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]

    def bitrshift(self):

        try:
            self.solution = self.equation.op1 >> self.equation.op2
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]

    def bitand(self):

        try:
            self.solution = self.equation.op1 & self.equation.op2
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]

    def bitor(self):

        try:
            self.solution = self.equation.op1 | self.equation.op2
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]

    def bitxor(self):

        try:
            self.solution = self.equation.op1 ^ self.equation.op2
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]

    def bitlrot(self):

        try:
            self.solution = (self.equation.op1 << self.equation.op2) | (self.equation.op1 >> (64 - self.equation.op2))
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]


    def bitrrot(self):

        try:
            self.solution = (self.equation.op1 >> self.equation.op2) | (self.equation.op1 << (64 - self.equation.op2)) & (2 ** 64 - 1)
        except:
            self.solution = 0
            self.eqflag = self.eqflags["ERROR"]
        

class EquGrader():

    def __init__(self, infile, solfile):

        with open(infile, "rb") as infile:
            self.infilebuf = infile.read()

        with open(solfile, "rb") as solfile:
            self.solfilebuf = solfile.read()

        self.fail = 0
        self.fulleqlen = SER_EQU_SIZE
        self.solutionlen = SOLV_EQU_SIZE
        self.inheader = UnpackedHeader(self.infilebuf)
        self.solheader = UnpackedHeader(self.solfilebuf)
        self.unsolvedlist = self.get_unsolved()
        self.givensols = self.get_solved()

        if self.inheader.numequ != len(self.givensols):
            print("Error: Number of equations in solution does not match header")
            self.fail = 1
            return self.fail

        self.checksols = self.solve_all()
        self.errors = self.get_results()
        if len(self.errors) > 0:
            self.fail += 1
            for i in range(0, len(self.errors)):
                print(self.errors[i])

    def get_unsolved(self):

        equations = []
        for i in range(0, self.inheader.numequ):
            start = (i * self.fulleqlen) + self.inheader.offset
            end = start + self.fulleqlen
            equations.append(Equation(self.infilebuf[start:end]))
        
        return equations


    def get_solved(self):

        solutions = []
        for i in range(self.inheader.numequ):
            start = (i * self.solutionlen) + self.inheader.offset
            end = start + self.solutionlen
            solutions.append(Solution(self.solfilebuf[start:end]))
            # print(solutions[i])
        
        return solutions

    def solve_all(self):

        solutions = []
        for i in range(0, len(self.unsolvedlist)):
            solutions.append(Solution(self.unsolvedlist[i]))

        return solutions


    

    def get_results(self):
        errors = []
        debug = []

        solsgiven = len(self.givensols)
        solsshouldbe = len(self.checksols)

        debug.append("[+] File headers. Got {}, should be {}.".format(self.inheader, self.solheader))
        if (self.inheader.magic != self.solheader.magic or
            self.inheader.fileid != self.solheader.fileid or
            self.inheader.numequ != self.solheader.numequ or
            self.inheader.offset != self.solheader.offset or
            self.inheader.numopt != self.solheader.numopt):
            errors.append("[-] File headers don't match.") 
            errors.append("[-] Expected:\n{}\n[-] Got:\n{}".format(self.inheader, self.solheader))
            debug.append("[+] Expected:\n{}\n[-] Got:\n{}".format(self.inheader, self.solheader))

        if solsgiven != solsshouldbe:
            errors.append("[-] Number of solutions don't match. Got {}, should be {}.".format(solsgiven, solsshouldbe))
        else:
            for i in range(len(self.givensols)):
                if (self.givensols[i].id != self.checksols[i].id or 
                    self.givensols[i].eqflag != self.checksols[i].eqflag or 
                    self.givensols[i].solution  !=  self.checksols[i].solution or
                    self.givensols[i].data_code != self.checksols[i].data_code):
                    if (self.givensols[i].solution  == self.checksols[i].solution):
                        continue
                    errors.append("[-] Calculation error.")
                    errors.append("{}".format(self.unsolvedlist[i]))
                    errors.append("[-] Expected:\n{}\n[-] Got:\n{}".format(self.checksols[i], self.givensols[i]))
                    debug.append("[+] Expected:\n{}\n[-] Got:\n{}".format(self.checksols[i], self.givensols[i]))
        
        # for i in range(0, len(debug)):
        #     print(debug[i])

        return errors


def matchfiles(file1, file2):
    """Ensure the files have the same file id"""
    with open(file1, "rb") as f1:
        buf = f1.read()
        h1 = UnpackedHeader(buf)
    with open(file2, "rb") as f2:
        buf = f2.read()
        h2 = UnpackedHeader(buf)

    if h1.fileid == h2.fileid:
        return True


def checkheader(filepath):
    magic = EQU_HDR_MAGIC
    notequ = -1
    with open(filepath, "rb") as file:
        buf = file.read()
        infilemagic = struct.unpack_from("<I", buf, offset=0)[0]
        infileflag = buf[20]
        if infilemagic == magic:
            return infileflag
        return notequ

def setup(dir, num_files, num_equ):

    indir = f"{dir}/unsolved"
    outdir = f"{dir}/solved"

    if os.path.isdir(dir):
        shutil.rmtree(dir)
    os.mkdir(dir)

    if os.path.isdir(indir):
        shutil.rmtree(indir)
    os.mkdir(indir)

    if os.path.isdir(outdir):
        shutil.rmtree(outdir)
    os.mkdir(outdir)

    generate_files(num_files, num_equ, indir)

def cleanup(tests_path):
    if os.path.isdir(tests_path):
        shutil.rmtree(tests_path)

def grade_dirs(dir):

    indir = f"{dir}/unsolved"
    outdir = f"{dir}/solved"

    num_files = 0
    num_failed = 0
    infilelist = []
    outfilelist = []

    for dirpath, _, filenames in os.walk(indir):
        for file in filenames:
            filepath = os.path.abspath(os.path.join(dirpath, file))
            if checkheader(filepath) == 0:
                infilelist.append(filepath)

    for dirpath, _, filenames in os.walk(outdir):
        for file in filenames:
            filepath = os.path.abspath(os.path.join(dirpath, file))
            if checkheader(filepath) == 1:
                outfilelist.append(filepath)

    for i in range(0, len(infilelist)):
        num_files += 1
        for j in range(0,len(outfilelist)):
            if matchfiles((infilelist[i]), (outfilelist[j])): #Comparing filenames
                num_failed += EquGrader(infilelist[i], outfilelist[j]).fail

    if (num_files != len(outfilelist)):
        num_failed += num_files - len(outfilelist)

    num_passed = num_files - num_failed
    print(f"Test Results: Passed {num_passed} out of {num_files} tests.")

    return((num_passed - num_files))

if __name__ == "__main__":
    pass