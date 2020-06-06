BOOST_ROOT := /usr/include/boost
SIMPLE := simple
POOL := pool

build:
	g++ ${SIMPLE}.cpp -o ${SIMPLE} -std=c++17 -I$(BOOST_ROOT) -lpthread -lcrypto -lboost_thread -lboost_chrono
	g++ ${POOL}.cpp -o ${POOL} -std=c++17 -I$(BOOST_ROOT) -lpthread -lcrypto -lboost_thread -lboost_chrono

clean:
	rm simple pool