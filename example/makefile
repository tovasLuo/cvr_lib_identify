#autocheck:autocheck.c
test:test.cpp
	g++ -fshort-wchar -L. -lwltlib -lTHID_CVR_IDCard test.cpp -o test ./libTHID_CVR_IDCard.a ./libwltlib.a #-lstdc++

#	g++ -fsanitize=address -g -fshort-wchar -L. -lwltlib -lTHID_CVR_IDCard autocheck.c -o autocheck ./libTHID_CVR_IDCard.a ./libwltlib.a #-lstdc++
clean:
#	rm -rf *.o autocheck	
	rm -rf *.o test
