bin/autoclicker: src/autoclicker.cpp
	g++ -Wall -O2 -lpthread -lX11 -o bin/autoclicker src/autoclicker.cpp
