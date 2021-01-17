TARGETS= piggy_back_trading_bot piggy_back_probability

CFLAGS= -Wall -g -O3 -std=c++2a -lcurl -lssl -lcrypto -lcpprest -lboost_system -lboost_thread-mt -lboost_chrono-mt  

CFLAGS2 = -I/usr/local/Cellar/nlohmann-json/3.9.1_1/include/nlohmann 

CFLAGS3= -I/usr/local/Cellar/cpprestsdk/2.10.16/include/ -I/usr/local/Cellar/boost/1.74.0/include/ -I/usr/local/opt/openssl@1.1/include 

LDFLAGS= -L/usr/lib  -L/usr/local/opt/openssl@1.1/lib -L/usr/local/Cellar/cpprestsdk/2.10.16/lib/ -L/usr/local/Cellar/boost/1.74.0/lib/

API-KEY-ID = 0

API-SECRET-KEY = 0

all: $(TARGETS)

piggy_back_probability: piggy_back_probability.cpp
ifneq (${API-KEY-ID}${API-SECRET-KEY}, 00)
	g++  $(CFLAGS) $(CFLAGS2) $(CFLAGS3) ${LDFLAGS} -o piggy_back_probability piggy_back_probability.cpp -DAPIKEYID=${API-KEY-ID} -DAPISECRETKEY=${API-SECRET-KEY}
else
	g++ $(CFLAGS) $(CFLAGS2) $(CFLAGS3) ${LDFLAGS} -o piggy_back_probability piggy_back_probability.cpp 
endif

piggy_back_trading_bot: piggy_back_trading_bot.cpp
ifneq (${API-KEY-ID}${API-SECRET-KEY}, 00)
	g++  $(CFLAGS) $(CFLAGS2) $(CFLAGS3) ${LDFLAGS} -o piggy_back_trading_bot piggy_back_trading_bot.cpp -DAPIKEYID=${API-KEY-ID} -DAPISECRETKEY=${API-SECRET-KEY}
else
	g++ $(CFLAGS) $(CFLAGS2) $(CFLAGS3) ${LDFLAGS} -o piggy_back_trading_bot piggy_back_trading_bot.cpp 
endif

clean:
	rm -rf $(TARGETS) *.dSYM