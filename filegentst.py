import random
def createRandomFile(oom_in_bytes, path):
    f = open(path,'wb')
    numBytes = random.randint(1,9) * oom_in_bytes + random.randint(1,oom_in_bytes)
    for i in range(numBytes):
        rbyte = random.randint(0,255)
        f.write(chr(rbyte))
    f.close()






import os
executable1 = "./xor_threads"
executable2 = "./xor_no_threads"
IN_FILES_DIRECTORY = "./testFiles/inFiles"

TEST_SIZE = 5
OUTFILE1 = "./testFiles/resultFileThreads.txt"
OUTFILE2 = "./testFiles/resultFileNoThreads.txt"

PRINT=False
print "Starting test:"
for i in range(TEST_SIZE):
	oom_in_bytes = random.choice([1,10,10**2,10**3, 10**6])
	numFiles= random.randint(1,12)
	for j in range(numFiles):
		fname= IN_FILES_DIRECTORY+"/"+str(j)
		createRandomFile(oom_in_bytes,fname)
		if PRINT:
			print "created file {} . oombytes = {}".format(fname,oom_in_bytes)
	IN_FILES =[ IN_FILES_DIRECTORY+"/" + f for f in os.listdir(IN_FILES_DIRECTORY)]
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
	if (0<TEST_SIZE<=20) or (20<TEST_SIZE<=200 and i%10==0) or (200<TEST_SIZE<=1000 and i%20==0) or (1000<TEST_SIZE and i%100==0) :	print "test {} succeeded. oom={}".format(i,oom_in_bytes)


