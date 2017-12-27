import os

IN_FILES_DIRECTORY = "./testFiles/inFiles"
IN_FILES =[ IN_FILES_DIRECTORY+"/" + f for f in os.listdir(IN_FILES_DIRECTORY)]
TEST_SIZE = 2
OUTFILE1 = "resultFileThreads.txt"
OUTFILE2 = "resultFileNoThreads.txt"

for i in range(TEST_SIZE):
   	os.system("./xor_threads  {} ".format(OUTFILE1)+" ".join(IN_FILES))
   	os.system("./xor_no_threads  {} ".format(OUTFILE2)+" {}/".format(IN_FILES_DIRECTORY).join(IN_FILES))

   	threadsRes = open(OUTFILE1,'rb').read()
   	noThreadsRes = open(OUTFILE2,'rb').read()
	#threadsRes =  threadsRes.read() 
	#noThreadsRes = noThreadsRes.read()

	
	if threadsRes != noThreadsRes:
		print "test #{} failed: (threads: {}) vs (no threads: {})".format(i,threadsRes,noThreadsRes)
		print "file sizes: threads: {} | no threads: {}".format(len(threadsRes),len(noThreadsRes))
