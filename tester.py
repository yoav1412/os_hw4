import os
executable1 = "./xor_threads"
executable2 = "./xor_no_threads"
IN_FILES_DIRECTORY = "./testFiles/inFiles"
IN_FILES =[ IN_FILES_DIRECTORY+"/" + f for f in os.listdir(IN_FILES_DIRECTORY)]
TEST_SIZE = 250
OUTFILE1 = "./testFiles/resultFileThreads.txt"
OUTFILE2 = "./testFiles/resultFileNoThreads.txt"



for i in range(TEST_SIZE):
   	os.system("{}  {} ".format(executable1, OUTFILE1)+" ".join(IN_FILES) + "> /dev/null")
   	os.system(" {} {} ".format(executable2, OUTFILE2)+ " ".join(IN_FILES) + "> /dev/null")

   	threadsRes = open(OUTFILE1,'rb').read()
   	noThreadsRes = open(OUTFILE2,'rb').read()

	#os.system("xxd -b " + OUTFILE1 + " > ./testFiles/binaries/binaryThreads" )
	#os.system("xxd -b " + OUTFILE2 + " > ./testFiles/binaries/binaryNoThreads" )
	#os.system("diff ./testFiles/binaries/binaryThreads  ./testFiles/binaries/binaryNoThreads" )

	if threadsRes != noThreadsRes:
		#print "test #{} failed: (threads: {}) vs (no threads: {})".format(i,threadsRes,noThreadsRes)
		print "file sizes: threads: {} | no threads: {}".format(len(threadsRes),len(noThreadsRes))
	if (0<TEST_SIZE<=20) or (20<TEST_SIZE<=200 and i%10==0) or (200<TEST_SIZE<=1000 and i%20==0) or (1000<TEST_SIZE and i%100==0) :	print "test {} succeeded.".format(i)


