bin/autoclicker: bin/autoclicker.o bin/log.o
	g++ -Wall -O2 -lpthread -lX11 -o bin/autoclicker bin/autoclicker.o bin/log.o

bin/autoclicker.o: src/autoclicker.cpp src/log.h
	g++ -Wall -O2 -c -o bin/autoclicker.o src/autoclicker.cpp

bin/log.o: src/log.cpp src/log.h
	g++ -Wall -O2 -c -o bin/log.o src/log.cpp


