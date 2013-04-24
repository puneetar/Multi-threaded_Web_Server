#  Makefile: Editing of Tim Budd's "Makefile" by Bina Ramamurthy and 
#  Kenneth W. Regan, Fall 1996, and again by KWR in Spring 2000 & Fall 2009
#
#  Produces executable named "myhttpd".
#  Edited by Stacey Askey for CSE 521 Fall 2012 Project 1.
#
#  USAGE---enter at command prompt in directory: make -f makefile   or   make 

# This sets variables that the make files use instead of needing to write 
# each multiple times. You use $(VAR) to invoke. 
CC     = g++ -Wall     # -Wall mandated now

LIBS   = -lm
OBJ    = .o
RM     = rm -fr

#PROG   = main.o MySched.o ReadyQueue.o request.o 
#socketprog.o thread.o

# SYNTAX: Dependencies must be on same line as ":".  Actions must
# follow on succeeding line(s), and have at least one TAB indent.

# this line is required with whatever the final file will be named
# specifies what to make when a specific set of files is not specified
all:	myhttpd

# Invoke this via "make -f makefile clean".  No "-" before "clean"!
clean:
	$(RM) *$(OBJ)    # ISR/*$(OBJ)---etc. for any subdirectories

# This gives the name of the file that will be output from the command
# along with what file will be used and the dependencies of that file
# form is 
# OutpufileName.o: CppFileused.cpp Dependencies which can include other .cpp
# tab, not space  $(CC) -c NameOFCppfileonly.cpp
#socketprog.o: socketprog.cpp
#	$(CC) -c socketprog.cpp

#thread.o: thread.cpp
#	$(CC) -c thread.cpp

#request.o: request.cpp request.h
#	$(CC) -c request.cpp

#ReadyQueue.h: request.h

#ReadyQueue.o: ReadyQueue.cpp ReadyQueue.h MySched.h request.h
#	$(CC) -c ReadyQueue.cpp

#MySched.h: ReadyQueue.h

#MySched.o: MySched.cpp MySched.h ReadyQueue.h request.h
#	$(CC) -c MySched.cpp

main.o: main.cpp
	$(CC) -c main.cpp
#this outputs the executable,
# name of executable: ListofOfile.o
# tab, not space $(CC) -o nameofoutput listof.oFiles.o $(LIBS)
# I will probaby make a variable for the various file names so these can 
# remain one line each part, since there can sometimes be difficulites
# when the names extend to a second line
myhttpd: main.o 
	$(CC) -o myhttpd main.o -lpthread $(LIBS)

# The .o file with main should come before all the other object files in the
# final linking stage.

