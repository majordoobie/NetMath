import struct
import os
import socket
import time
import EquGrader

NET_HDR_SZ = 48
NET_FNAME_MAX = 24
NET_NAME_FIELD_SZ = 32


def gen_net_hdr(pkt_len, efile_name_len, efile_name):
    hdr = b""
    hdr_len = NET_HDR_SZ
    filename_len = efile_name_len
    pkt_len = pkt_len + NET_HDR_SZ
    filename = efile_name.ljust(NET_NAME_FIELD_SZ, "\x00")

    hdr = struct.pack("!IIQ", hdr_len, filename_len, pkt_len)
    hdr += filename.encode('utf-8')

    return hdr

INVALID_HDR_RSP = struct.pack("!IIQ", NET_HDR_SZ, 0, NET_HDR_SZ).ljust(NET_HDR_SZ, b"\x00") #sends back just the hdr size

def check_nethdr_handling():

    tests_passed = 0

    hdrs = [(gen_net_hdr(100, 40, "FAKE_FILE"), INVALID_HDR_RSP)]

    for h in hdrs:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(("127.0.0.1", 31337))
            s.send(h[0])
            data = s.recv(NET_HDR_SZ)
            if data == h[1]:
                tests_passed += 1
            else:
                print("Failed to Handle Hdr:", h[1], data)
    
    return tests_passed - len(hdrs)

def check_nethdr_correctness():
    correct_ret_hdr = (48, 20, 523)
    EquGrader.generate_files(1, 32, ".") #create temporary file

    ret = 1

    test_file = ""
    for i in os.listdir("."):
        # List files with .py
        if i.endswith("equ"):
            test_file = i

    with open(test_file, 'rb') as ef:
        edata = ef.read()
        hdr = gen_net_hdr(len(edata), len(test_file), test_file)
        pkt = hdr + edata
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.connect(("127.0.0.1", 31337))
                    s.sendall(pkt)
                    hdr = s.recv(NET_HDR_SZ)
                    uphdr = struct.unpack("!IIQ", hdr[:16])
                    if uphdr != correct_ret_hdr:
                        ret = -1

    os.system("rm ./*.equ") #remove temporary file

    return ret
        
        
def run_binary():
    os.chdir("./5_NetCalc")
    os.system("./build.sh")
    print("Starting server in background")
    os.system("./build/netcalc &") # start server in background
    time.sleep(1) #ensure server spinup
    try:
        if check_nethdr_handling() < 0:
            print("Failed to correctly handle bad NetHdrs; Aborting further tests")
            os.system("pkill netcalc")
            exit(-100)
    except ConnectionRefusedError:
        print("Failed to connect to server; Aborting further tests")
        exit(-100)

    try:
        if check_nethdr_correctness() < 0:
            print("Recived Invalid NetHdr from server; Aborting further tests")
            os.system("pkill netcalc")
            exit(-100)
    except ConnectionRefusedError:
        print("Failed to connect to server; Aborting further tests")
        os.system("pkill netcalc")
        exit(-100)   
    
    if not os.path.exists("client/client.py"):
        print("client.py not found; verify location")
        os.system("pkill netcalc")
        exit(-100)

    os.system("python3 client/client.py -i ../netcalc_tests/unsolved -o ../netcalc_tests/solved")
    os.system("pkill netcalc")
    os.chdir("../")


def main():
    result = 0
    tests_base =  "./netcalc_tests/"
    num_files = 16
    num_equ = 64

    EquGrader.setup(tests_base, num_files, num_equ)
    run_binary()
    result = EquGrader.grade_dirs(tests_base)
    EquGrader.cleanup(tests_base)

    return result


if __name__ == "__main__":
    main()
