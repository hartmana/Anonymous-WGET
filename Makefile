awget: 	awget.o TCPServerSocket.o TCPSocket.o StringUtils.o 
	g++ -std=c++11 -lpthread -I. -Wall *.o -o awget

ss:	ss.o TCPServerSocket.o TCPSocket.o StringUtils.o NetworkUtils.o
	g++ -std=c++11 -lpthread -I. -Wall *.o -o ss

TCPServerSocket.o:	TCPServerSocket.h TCPServerSocket.cpp
			g++ -std=c++11 -lpthread -I. -Wall -c TCPServerSocket.cpp

TCPSocket.o:	TCPSocket.h TCPSocket.cpp
		g++ -std=c++11 -lpthread -I. -Wall -c TCPSocket.cpp

StringUtils.o:	StringUtils.h StringUtils.cpp
		g++ -std=c++11 -lpthread -I. -Wall -c StringUtils.cpp

NetworkUtils.o: NetworkUtils.h
		g++ -lpthread -I. -Wall -c NetworkUtils.h

main.o:	main.cpp 
	g++ -std=c++11 -lpthread -I. -Wall -c main.cpp

awget.o: awget.cpp
	g++ -std=c++11 -lpthread -I. -Wall -c awget.cpp

ss.o:	ss.cpp
	g++ -std=c++11 -lpthread -I. -Wall -c ss.cpp

clean:	
	rm -f *.o
	rm -f awget
	rm -f ss

