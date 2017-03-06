# makefile
#
# Created time: 2011-7-14
# Author: Ly


OBJ_DIR = 
LDLIBS  = 

CC	  	  = g++ -I./include -I../include -O3
#CC	  	  = arm_v5t_le-gcc -I./include -I../include


OBJS 	  := THID_RS232.o THID_CVR_IDCard.o 
LIB 	  := 

#
##create static lib (.a)
#
libTHID_CVR_IDCard.a:	$(OBJS)
	ar rcs libTHID_CVR_IDCard.a $(OBJS)
	
#
#create shared lib (.so)
# -fsigned-char 
#	$(CC) -Wl,-soname,libcard.so -o libcard.so $(OBJS) libwltlib.a -O3

%.o : %.c
	$(CC) -c -O2 $<  -L./

TSTLIB  := libwltlib.a #lstdc++


al:
	$(CC)  -fshort-wchar -Wall -c ./example/CVR_IDCardLibTest.c -o CVR_IDCardLibTest.o #-fshort-wchar
	$(CC)  -fshort-wchar -o Test_CVR_Idcard_a CVR_IDCardLibTest.o $(TSTLIB) -lpthread #-fshort-wchar
	$(CC)  -fshort-wchar -L./  -o Test_CVR_Idcard_so CVR_IDCardLibTest.o -lcard -lpthread #-lstdc++ -fshort-wchar

#	cp Test_CVR_Idcard_a /tftpboot/filesys/ly/
#	cp Test_CVR_Idcard_so /tftpboot/filesys/ly/
	#sudo cp *.so /usr/lib/

	

.PHONY:	clean
clean:
	rm -rf *.o libTHID_CVR_IDCard.a Test_CVR_Idcard_a Test_CVR_Idcard_so

