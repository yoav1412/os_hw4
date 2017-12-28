import os
import random
executable1 = "./xor_threads"
executable2 = "./xor_threads_small_chunk"
executable3 = "./melman"
executable4 = "./xor_no_threads"

IN_FILES_DIRECTORY = "./testFiles/inFiles"
IN_FILES =[ IN_FILES_DIRECTORY+"/" + f for f in os.listdir(IN_FILES_DIRECTORY)]
TEST_SIZE = 5000
OUTFILE1 = "./testFiles/resultExec1.txt"
OUTFILE2 = "./testFiles/resultExec2.txt"
OUTFILE3 = "./testFiles/resultExec3.txt"
OUTFILE4 = "./testFiles/resultExec4.txt"


print IN_FILES
for i in range(TEST_SIZE):
	random.shuffle(IN_FILES)
	numFiles = random.randint(1,len(IN_FILES))
   	os.system("{}  {} ".format(executable1, OUTFILE1)+" ".join(IN_FILES[:numFiles]) + "> /dev/null")
   	os.system(" {} {} ".format(executable2, OUTFILE2)+ " ".join(IN_FILES[:numFiles]) + "> /dev/null")
	os.system(" {} {} ".format(executable3, OUTFILE3)+ " ".join(IN_FILES[:numFiles]) + "> /dev/null")
	os.system(" {} {} ".format(executable4, OUTFILE4)+ " ".join(IN_FILES[:numFiles]) + "> /dev/null")

   	threadsRes = open(OUTFILE1,'rb').read()
   	noThreadsRes = open(OUTFILE2,'rb').read()
   	melmanRes = open(OUTFILE3,'rb').read()
   	exec4Res = open(OUTFILE4,'rb').read()


	"""
	os.system("xxd -b " + OUTFILE1 + " > ./testFiles/binaries/binary_exec1" )
	os.system("xxd -b " + OUTFILE2 + " > ./testFiles/binaries/binary_exec2" )
	os.system("xxd -b " + OUTFILE3 + " > ./testFiles/binaries/binary_exec3" )
	os.system("xxd -b " + OUTFILE4 + " > ./testFiles/binaries/binary_exec4" )

	r1=os.system("diff ./testFiles/binaries/binary_exec1  ./testFiles/binaries/binary_exec2"+ "> /dev/null"  )
	r2=os.system("diff ./testFiles/binaries/binary_exec1  ./testFiles/binaries/binary_exec3"+ "> /dev/null" )
	r3=os.system("diff ./testFiles/binaries/binary_exec1  ./testFiles/binaries/binary_exec4"+ "> /dev/null" )
	if (r1 !=0 or r2 != 0):# or r3!=0):
		print "test {}  failed on binary diff check".format(i)
		print "test failed on files:"
		print IN_FILES[:numFiles]
		break
	"""
	if (threadsRes != noThreadsRes) or (melmanRes != threadsRes):
		print "test {} failed. file sizes: exec1: {} | exec2: {} | exec3 {}| exec4 {} ".format(i,len(threadsRes),len(noThreadsRes), len(melmanRes), len(exec4Res))
		print "test failed on files:"
		print IN_FILES[:numFiles]
		break
	elif (0<TEST_SIZE<=20) or (20<TEST_SIZE<=200 and i%10==0) or (200<TEST_SIZE<=1000 and i%20==0) or (1000<TEST_SIZE and i%100==0) :	
		print "test {} succeeded.".format(i)


