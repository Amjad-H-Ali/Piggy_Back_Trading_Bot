TARGETS= piggy_back_trading_bot piggy_back_probability

CFLAGS= -Wall -g -O3 -std=c++2a -L/usr/lib -lcurl -I/usr/local/Cellar/nlohmann-json/3.9.1_1/include/nlohmann

API-KEY-ID = 0

API-SECRET-KEY = 0

all: $(TARGETS)

piggy_back_probability: piggy_back_probability.cpp
ifneq (${API-KEY-ID}${API-SECRET-KEY}, 00)
	g++ $(CFLAGS) -o piggy_back_probability piggy_back_probability.cpp -DAPIKEYID=${API-KEY-ID} -DAPISECRETKEY=${API-SECRET-KEY}
else
	g++ $(CFLAGS) -o piggy_back_probability piggy_back_probability.cpp 
endif

piggy_back_trading_bot: piggy_back_trading_bot.cpp
ifneq (${API-KEY-ID}${API-SECRET-KEY}, 00)
	g++ $(CFLAGS) -o piggy_back_trading_bot piggy_back_trading_bot.cpp -DAPIKEYID=${API-KEY-ID} -DAPISECRETKEY=${API-SECRET-KEY}
else
	g++ $(CFLAGS) -o piggy_back_trading_bot piggy_back_trading_bot.cpp 
endif

clean:
	rm -rf $(TARGETS) *.dSYM