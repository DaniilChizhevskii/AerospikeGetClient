BOOST_ROOT := /usr/include/boost
PROGRAM := main

build: main.cpp
	g++ main.cpp -o ${PROGRAM} -std=c++17 -I$(BOOST_ROOT) -lpthread -lcrypto -lboost_thread -lboost_chrono

run:
	./${PROGRAM}

clean:
	rm -r build