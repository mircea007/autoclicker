CXX      = g++
CXXFLAGS = -Wall -O2
LDFLAGS  = -lpthread -lX11

bin/clickr: bin/main.o bin/log.o bin/autoclickers.o bin/mimic.o bin/os_specific.o | bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o bin/clickr bin/main.o bin/log.o bin/autoclickers.o bin/mimic.o

bin/main.o: src/main.cpp src/log.h src/mimic.h src/os_specific.h | bin
	$(CXX) $(CXXFLAGS) -c -o bin/main.o src/main.cpp

# MimicMouseButFaster
bin/mimic.o: src/mimic.cpp src/mimic.h src/log.h src/autoclickers.h src/os_specific.h | bin
	$(CXX) $(CXXFLAGS) -c -o bin/mimic.o src/mimic.cpp

# AsyncAutoClicker, SyncAutoClicker
bin/autoclickers.o: src/autoclickers.cpp src/log.h src/autoclickers.h src/os_specific.h | bin
	$(CXX) $(CXXFLAGS) -c -o bin/autoclickers.o src/autoclickers.cpp

# library that abstracts away a lot of os-specific tasks
bin/os_specific.o: src/os_specific.cpp src/log.h src/os_specific.h | bin
	$(CXX) $(CXXFLAGS) -c -o bin/os_specific.o src/os_specific.cpp

# simple logging library
bin/log.o: src/log.cpp src/log.h | bin
	$(CXX) $(CXXFLAGS) -c -o bin/log.o src/log.cpp

bin:
	mkdir bin