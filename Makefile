TARGETS= piggy_back_trading_bot

CFLAGS= -Wall -g -O3 -std=c++2a -L/usr/lib -lcurl -I/usr/local/Cellar/nlohmann-json/3.9.1_1/include/nlohmann

all: $(TARGETS)

piggy_back_trading_bot: piggy_back_trading_bot.cpp
	g++ $(CFLAGS) -o piggy_back_trading_bot piggy_back_trading_bot.cpp

clean:
	rm -f $(TARGETS)