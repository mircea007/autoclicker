bin/autoclicker: bin/main.o bin/log.o bin/autoclickers.o bin/mimic.o
	g++ -Wall -O2 -lpthread -lX11 -o bin/autoclicker bin/main.o bin/log.o bin/autoclickers.o bin/mimic.o

bin/main.o: src/main.cpp src/log.h src/mimic.h
	g++ -Wall -O2 -c -o bin/main.o src/main.cpp

# MimicMouseButFaster
bin/mimic.o: src/mimic.cpp src/mimic.h src/log.h src/autoclickers.h
	g++ -Wall -O2 -c -o bin/mimic.o src/mimic.cpp

# AsyncAutoClicker, SyncAutoClicker
bin/autoclickers.o: src/autoclickers.cpp src/log.h src/autoclickers.h
	g++ -Wall -O2 -c -o bin/autoclickers.o src/autoclickers.cpp

# simple logging library
bin/log.o: src/log.cpp src/log.h
	g++ -Wall -O2 -c -o bin/log.o src/log.cpp

