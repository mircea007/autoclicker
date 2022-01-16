CXX      = g++
CXXFLAGS = -Wall -O2
LDFLAGS  = -lpthread -lX11

bin/autoclicker: bin/main.o bin/log.o bin/autoclickers.o bin/mimic.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/clickr bin/main.o bin/log.o bin/autoclickers.o bin/mimic.o

bin/main.o: src/main.cpp src/log.h src/mimic.h
	$(CXX) $(CXXFLAGS) -c -o bin/main.o src/main.cpp

# MimicMouseButFaster
bin/mimic.o: src/mimic.cpp src/mimic.h src/log.h src/autoclickers.h
	$(CXX) $(CXXFLAGS) -c -o bin/mimic.o src/mimic.cpp

# AsyncAutoClicker, SyncAutoClicker
bin/autoclickers.o: src/autoclickers.cpp src/log.h src/autoclickers.h
	$(CXX) $(CXXFLAGS) -c -o bin/autoclickers.o src/autoclickers.cpp

# simple logging library
bin/log.o: src/log.cpp src/log.h
	$(CXX) $(CXXFLAGS) -c -o bin/log.o src/log.cpp

