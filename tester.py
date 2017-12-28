import os
import random
executable1 = "./xor_threads"
executable2 = "./xor_threads_small_chunk"
executable3 = "./melman"

IN_FILES_DIRECTORY = "./testFiles/morefiles"
IN_FILES =[ IN_FILES_DIRECTORY+"/" + f for f in os.listdir(IN_FILES_DIRECTORY)]
TEST_SIZE = 5000
OUTFILE1 = "./testFiles/resultFileThreads.txt"
OUTFILE2 = "./testFiles/resultFileNoThreads.txt"
OUTFILE3 = "./testFiles/resultFileMelman.txt"


print IN_FILES
for i in range(TEST_SIZE):
	random.shuffle(IN_FILES)
	numFiles = random.randint(1,len(IN_FILES))
   	os.system("{}  {} ".format(executable1, OUTFILE1)+" ".join(IN_FILES[:numFiles]) + "> /dev/null")
   	os.system(" {} {} ".format(executable2, OUTFILE2)+ " ".join(IN_FILES[:numFiles]) + "> /dev/null")
	os.system(" {} {} ".format(executable3, OUTFILE3)+ " ".join(IN_FILES[:numFiles]) + "> /dev/null")

   	threadsRes = open(OUTFILE1,'rb').read()
   	noThreadsRes = open(OUTFILE2,'rb').read()
   	melmanRes = open(OUTFILE3,'rb').read()

	#os.system("xxd -b " + OUTFILE1 + " > ./testFiles/binaries/binaryThreads" )
	#os.system("xxd -b " + OUTFILE2 + " > ./testFiles/binaries/binaryNoThreads" )
	#os.system("diff ./testFiles/binaries/binaryThreads  ./testFiles/binaries/binaryNoThreads" )

	if (threadsRes != noThreadsRes) or (melmanRes != threadsRes):
		#print "test #{} failed: (threads: {}) vs (no threads: {})".format(i,threadsRes,noThreadsRes)
		print "test {} failed. file sizes: threads: {} | no threads: {} | third {} ".format(i,len(threadsRes),len(noThreadsRes), len(melmanRes))
	if (0<TEST_SIZE<=20) or (20<TEST_SIZE<=200 and i%10==0) or (200<TEST_SIZE<=1000 and i%20==0) or (1000<TEST_SIZE and i%100==0) :	print "test {} succeeded.".format(i)


