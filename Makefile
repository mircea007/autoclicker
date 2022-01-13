bin/autoclicker: bin/main.o bin/log.o bin/autoclickers.o
	g++ -Wall -O2 -lpthread -lX11 -o bin/autoclicker bin/main.o bin/log.o bin/autoclickers.o

bin/main.o: src/main.cpp src/log.h src/autoclickers.h
	g++ -Wall -O2 -c -o bin/main.o src/main.cpp

bin/autoclickers.o: src/autoclickers.cpp src/log.h src/autoclickers.h
	g++ -Wall -O2 -c -o bin/autoclickers.o src/autoclickers.cpp

bin/log.o: src/log.cpp src/log.h
	g++ -Wall -O2 -c -o bin/log.o src/log.cpp

