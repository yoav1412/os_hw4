import subprocess

IN_FILES = ["in1","in2"]
TEST_SIZE = 10
for i in range(TEST_SIZE):
    subprocess.call("xor_threads.txt resultFileThreads "+" ".join(IN_FILES))
    subprocess.call("xor_no_threads.txt resultFileNoThreads "+" ".join(IN_FILES))

    threadsRes = open('xor_threads.txt','rb')
    noThreadsRed = open('xor_no_threads.txt','rb')
    if threadsRes.read() != noThreadsRed.read():
        print "test #{} failed.".format(i)
