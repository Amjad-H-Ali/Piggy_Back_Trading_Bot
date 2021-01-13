#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <json.hpp>
#include <time.h>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <thread>



// For convenience
using json = nlohmann::json;

#define STR(X) #X
#define STRINGIZE_VAL(X) STR(X)
#define START_MONTH "01"
#define START_DAY 	"12"
#define START_YEAR  "2021"
#define NOW (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
#define HOUR 9
#define MIN 41
#define NEXT_HOUR 9
#define NEXT_MINUTE 42

// Appends response to given output string
static size_t write_data(const char* in, std::size_t size, std::size_t num, char* out) {

	// Append response to output string.
	((std::string*)out)->append(in, size*num);

	return size * num;
}

uint64_t fulfill_post_request(const std::string& request, std::string& response, const std::string& parameters) {

	// Set HTTP header
	static CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, request.c_str());
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,20);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set keys needed for API to authenticate 
	struct curl_slist *keys = NULL;
	keys = curl_slist_append(keys, "APCA-API-KEY-ID: " STRINGIZE_VAL(APIKEYID));
	keys = curl_slist_append(keys, "APCA-API-SECRET-KEY: " STRINGIZE_VAL(APISECRETKEY));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, keys);

	// The body or parameters of the Post request
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

	// Response information
	uint64_t http_status_code = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    
	response = "";

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);

	// curl_easy_cleanup(curl);

	return http_status_code;
}

uint64_t fulfill_delete_request(const std::string& request, std::string& response, const std::string& parameters) {

	// Set HTTP header
	static CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, (request+parameters).c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,20);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set keys needed for API to authenticate 
	struct curl_slist *keys = NULL;
	keys = curl_slist_append(keys, "APCA-API-KEY-ID: " STRINGIZE_VAL(APIKEYID));
	keys = curl_slist_append(keys, "APCA-API-SECRET-KEY: " STRINGIZE_VAL(APISECRETKEY));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, keys);

	// The body or parameters of the Post request
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());

	// Response information
	uint64_t http_status_code = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    
	response = "";

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);

	// curl_easy_cleanup(curl);

	return http_status_code;
}


// Given a string request, does an HTTP request to a server and stores
// response in given string response object.
uint64_t fulfill_get_request(const std::string& request, std::string& response) {

	// Set HTTP header
	static CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, request.c_str());
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,20);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set keys needed for API to authenticate 
	struct curl_slist *keys = NULL;
	keys = curl_slist_append(keys, "APCA-API-KEY-ID: " STRINGIZE_VAL(APIKEYID));
	keys = curl_slist_append(keys, "APCA-API-SECRET-KEY: " STRINGIZE_VAL(APISECRETKEY));
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, keys);

	// Response information
	uint64_t http_status_code = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    
	response = "";

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);

	// curl_easy_cleanup(curl);

	return http_status_code;
}

// Convert a given date to a Unix time in ms.
time_t get_time_in_ms(uint64_t month, uint64_t day, uint64_t year, uint64_t hour, uint64_t min, uint64_t sec) {

	struct tm *tminfo;

	time_t rawtime;

	time(&rawtime);

	tminfo = gmtime(&rawtime);

	tminfo->tm_year = year-1900;

	tminfo->tm_mon  = month;

	tminfo->tm_mday = day;

	tminfo->tm_hour = hour;

	tminfo->tm_min  = min;

	tminfo->tm_sec  = sec;

	return  mktime(tminfo)*1000;
}

struct Prev_Market_Data_T{

	float price,
		  volume;

	Prev_Market_Data_T(float Price, float Volume) 
		:price(Price), volume(Volume) {}
};

int main(void) {

#if defined (APIKEYID) &&  defined (APISECRETKEY)

	// Request market data for previous open day
	std::string request = "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/" START_YEAR "-" START_MONTH "-" START_DAY "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

	std::string response;

	fulfill_get_request(request, response);

	decltype(json::parse(response)) market_data_json;


	

	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Parse error parsing grouped data for previous open day" << std::endl;

		}
		catch(nlohmann::detail::type_error err) {

			std::cerr << "Type error parsing grouped data for previous open day" << std::endl;

			break;
		}
	}

	
std::cout << "01" << std::endl;


	// Insert previous market data, ticker, closing price and volume, into a map
	std::map<std::string, Prev_Market_Data_T> prev_market_data_map;

	for(uint64_t stock_indx = 0, stock_count = market_data_json["resultsCount"]; stock_indx < stock_count; ++stock_indx) {

		// Ignore corrupt data
		if(static_cast<std::string>(market_data_json["results"][stock_indx]["T"]).size() > 10) continue;

		prev_market_data_map.emplace(

			static_cast<std::string>(market_data_json["results"][stock_indx]["T"]),

			Prev_Market_Data_T(

				static_cast<float>(market_data_json["results"][stock_indx]["c"]), 

				static_cast<float>(market_data_json["results"][stock_indx]["v"])
			)
			
		);
	}
std::cout << "02" << std::endl;

	// Put thread to sleep until next market open
	uint64_t ms_till_open = get_time_in_ms(0, 13, 2021, HOUR, MIN, 5) - NOW;

	std::this_thread::sleep_for(std::chrono::milliseconds(ms_till_open));

	// Request current market data
	request = "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/2021-01-13?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

	fulfill_get_request(request, response);

	
	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Parse error parsing grouped data for current open day" << std::endl;

		}
		catch(nlohmann::detail::type_error err) {

			std::cerr << "Type error parsing grouped data for current open day" << std::endl;

			break;
		}
	}

	std::cout << "03" << std::endl;

	std::vector<std::string> watchlist;

	std::map<std::string, Prev_Market_Data_T>::iterator it_end = prev_market_data_map.end();
	// If stock has a relative volume (from last trading day) spike of 500% or more, and has a spike in price of 5% or more, then trade stock
	// when its price reaches a price 2% (or more) below the VWAP. Sell the stock if the price declines below the last low of the minute ticker.
	for(uint64_t stock_indx = 0, stock_count = market_data_json["resultsCount"]; stock_indx < stock_count; ++stock_indx) {

		std::map<std::string, Prev_Market_Data_T>::iterator it_prev_market = prev_market_data_map.find(static_cast<std::string>(market_data_json["results"][stock_indx]["T"]));

		// Previous data not found
		if(it_prev_market == it_end) continue;

		// Compute percent change from previous day
		float price_percent_change =  (static_cast<float>(market_data_json["results"][stock_indx]["o"]) - (it_prev_market->second).price)/(it_prev_market->second).price,

   			  volume_percent_change = (static_cast<float>(market_data_json["results"][stock_indx]["v"]) - (it_prev_market->second).volume)/(it_prev_market->second).volume;

		// Stock's price spiked 5% or more, volume spiked 500% or more, total day's volume of last trading day was 150k or more.
		if((price_percent_change >= 0.05) && (volume_percent_change >= 5) && ((it_prev_market->second).volume >= 150000)) {

			watchlist.emplace_back(static_cast<std::string>(market_data_json["results"][stock_indx]["T"]));
		}
	}


std::cout << "04" << std::endl;

	request = "https://paper-api.alpaca.markets/v2/account";

	fulfill_get_request(request, response);

	
	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Parse error while parsing cash" << std::endl;

		}
		catch(nlohmann::detail::type_error err) {

			std::cerr << "Type error while parsing cash" << std::endl;

		}
	}

	std::cout << "05" << std::endl;

	if (watchlist.size() == 0) {

		int64_t ms_till_tomorrow = get_time_in_ms(0, 14, 2021, 8, 30, 5) - NOW;

		std::this_thread::sleep_for(std::chrono::milliseconds(ms_till_tomorrow));
	}
	std::cout << "06" << std::endl;


	float capital_distribution = std::min(std::stof(static_cast<std::string>(market_data_json["cash"]))/watchlist.size(), std::stof(static_cast<std::string>(market_data_json["cash"]))*static_cast<float>(0.05));
	std::map<std::string, bool> open_position_map, buy_order_map, sell_order_map;

	std::string watchlist_tickers = "";

	for(const std::string& ticker : watchlist) {

		watchlist_tickers += (ticker + ",");

		open_position_map[ticker] = false, buy_order_map[ticker] = false, sell_order_map[ticker] = false;
	}

	// Delete last comma
	watchlist_tickers.erase(watchlist_tickers.end()-1);

	
std::cout << "07" << std::endl;

	// Put thread to sleep until next minute
	int64_t ms_till_start = get_time_in_ms(0, 13, 2021, NEXT_HOUR, NEXT_MINUTE, 5) - NOW;

	if(ms_till_start > 0) {

		std::this_thread::sleep_for(std::chrono::milliseconds(ms_till_start));
	}

	std::cout << "08" << std::endl;

	uint64_t last_minute = NOW, request_limit = 0;

	while(true) {




		request = "https://paper-api.alpaca.markets/v2/positions";
		fulfill_get_request(request, response);
		++request_limit;

		// Attempt to parse response into json
		while(true) {

			try {

				market_data_json = json::parse(response);

				break;
			}
			catch(nlohmann::detail::parse_error err) {

				std::cerr << "Parse error while parsing positions" << std::endl;

			}
			catch(nlohmann::detail::type_error err) {

				std::cerr << "Type error while parsing positions" << std::endl;

			}
		}

std::cout << "09" << std::endl;
		uint64_t position_indx = 0;
		while(market_data_json[position_indx] != nullptr) {

			std::string ticker = market_data_json[position_indx]["symbol"];

			open_position_map[ticker] = true;
			buy_order_map[ticker]     = false;
			
			++position_indx;
		}


		request = "https://api.polygon.io/v2/snapshot/locale/us/markets/stocks/tickers?tickers=" + watchlist_tickers + "&apiKey=" STRINGIZE_VAL(APIKEYID);


		fulfill_get_request(request, response);
		++request_limit;


		// Attempt to parse response into json
		while(true) {

			try {

				market_data_json = json::parse(response);

				break;
			}
			catch(nlohmann::detail::parse_error err) {

				std::cerr << "Parse error while parsing snap shot of all stocks" << std::endl;

			}
			catch(nlohmann::detail::type_error err) {

				std::cerr << "Type error while parsing snap shot of all stocks" << std::endl;

			}
		}

std::cout << "10" << std::endl;

		for(uint64_t stock_indx = 0, stock_count = market_data_json["count"]; stock_indx < stock_count; ++stock_indx) {

			std::string ticker = market_data_json["tickers"][stock_indx]["ticker"];


std::cout << "11" << std::endl;

			if(!open_position_map[ticker] && !buy_order_map[ticker]) {

				
				float limit_price = static_cast<float>(market_data_json["tickers"][stock_indx]["min"]["vw"])*static_cast<float>(0.98);
				uint64_t qty = capital_distribution/limit_price;
				request = "https://paper-api.alpaca.markets/v2/orders";

				decltype(json::parse(response)) param_json;


std::cout << "12" << std::endl;

				param_json["side"]        		= "buy";
				param_json["symbol"]      		= ticker;
				param_json["type"]   	  		= "limit";
				param_json["limit_price"] 		= limit_price;
				param_json["qty"]         		= qty;
				param_json["time_in_force"]     = "gtc";


std::cout << "13" << std::endl;

				uint64_t status_code = fulfill_post_request(request, response, param_json.dump());
				++request_limit;

				while((status_code == 403) && ((qty/=2) >= 1)) {

					std::cerr << "Insuffucient Buying Power. Decreasing qty" << std::endl;

					param_json["qty"] = qty;

					status_code = fulfill_post_request(request, response, param_json.dump());
					++request_limit;

std::cout << "14" << std::endl;
				}

				if(status_code != 200) {
					std::cerr << "Non OK status when submitting buy order for " << ticker << ". Status was: " << status_code << std::endl;

					continue;
				}

				sell_order_map[ticker]  = false;
				buy_order_map[ticker]   = true;
	
			}
		}

		request = "https://paper-api.alpaca.markets/v2/positions";
		fulfill_get_request(request, response);
		++request_limit;

		// Attempt to parse response into json
		while(true) {

			try {

				market_data_json = json::parse(response);

				break;
			}
			catch(nlohmann::detail::parse_error err) {

				std::cerr << "Parse error while parsing positions" << std::endl;

			}
			catch(nlohmann::detail::type_error err) {

				std::cerr << "Type error while parsing positions" << std::endl;

			}
		}


		uint64_t position_indx2 = 0;
		while(market_data_json[position_indx2] != nullptr) {


std::cout << "15" << std::endl;

			std::string ticker = market_data_json[position_indx2]["symbol"];
			
			buy_order_map[ticker]  = false;
	

			if(sell_order_map[ticker] == false) {


				request = "https://paper-api.alpaca.markets/v2/orders";

			
				decltype(json::parse(response)) param_json;

				param_json["side"]        		= "sell";
				param_json["symbol"]      		= ticker;
				param_json["type"]   	  		= "trailing_stop";
				param_json["trail_percent"]     = "0.5";
				param_json["qty"]         		= market_data_json[position_indx2]["qty"];
				param_json["time_in_force"]     = "gtc";

				uint64_t status_code = fulfill_post_request(request, response, param_json.dump());
				++request_limit;


std::cout << "16" << std::endl;

				if(status_code != 200) {
					std::cerr << "Non OK status when submitting sell order for " << ticker << ". Status was: " << status_code << std::endl;
					std::cerr << param_json.dump(4) << std::endl;
				}

				sell_order_map[ticker] = true;
				
			}

			
			++position_indx2;
		}


		for(auto& open_position_pair: open_position_map) {
			open_position_pair.second = false;
		}


		if((NOW - last_minute) >= 60000) {

			request_limit = 0;

			request = "https://paper-api.alpaca.markets/v2/orders";

			fulfill_get_request(request, response);
			++request_limit;
 

			while(1) {

				try {

					market_data_json = json::parse(response);

					break;

				}
				catch(nlohmann::detail::parse_error err) {

					std::cerr << "Parse error while getting list of orders. Could not update minute" << std::endl;

				}
				catch(nlohmann::detail::type_error err) {

					std::cerr << "Type error while getting list of orders. Could not update minute" << std::endl;

				}
			}

			request = "https://paper-api.alpaca.markets/v2/orders";
	
			uint64_t order_indx = 0;
			while(market_data_json[order_indx] != nullptr) {


std::cout << "17" << std::endl;

				if(static_cast<std::string>(market_data_json[order_indx]["side"]) == static_cast<std::string>("buy")) {
					std::string order_id = "/" + static_cast<std::string>(market_data_json[order_indx]["id"]);

					fulfill_delete_request(request, response, order_id);
					++request_limit;
				}


std::cout << "18" << std::endl;

				++order_indx;
			}

			for(auto& buy_order_pair: buy_order_map) {
				buy_order_pair.second = false;
			}

			last_minute = NOW;

		}
std::cout << "19" << std::endl;

		uint64_t ms_to_prevent_request_limit = 0;
		if((request_limit >= 180) && (( ms_to_prevent_request_limit = (NOW - last_minute)) < 60000)) {
			std::cout << "Sleeping to prevent request limit" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(std::max(60000 - ms_to_prevent_request_limit, static_cast<uint64_t>(0)) ));
		}



	}
	

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif

}









	// Distribute even amount of capital for each stock in watchlist

		// std::min(50k/watchlist.size(), 2500)

	// Create snap shot request string with all stocks in watchlist

	// Insert all stocks in watchlist in all maps with ticker and bool set to false
		
	// Wait till 8:31:05 AM

	// While true:

		// Request positions

		// For each position:

			// Set open_position_map[position.symbol] = true

			// Set buy_order_map[position.symbol] = false


		// Request snap shot of stocks in watchlist

		// For each stock in Response object:

			// if (open_position_map[position.symbol] == false) and (buy_order_map[position.symbol] == false):

				// Calculate 2% below VWAP 

				// Divide capital buy that price

				// Sumbit a limit order for 2% below vwap with correct qty

				// Set sell_order_map[position.symbol] = false

				// Set buy_order_map[position.symbol] = true


		// Request positions

		// For each position:
		
			// If sell_order_map[position.symbol] == false:

				// Submit a trailing stop order with a trail_percent of 0.5% for the correct qty

				// Set sell_order_map[position.symbol] = true

				// Set buy_order_map[position.symbol] = false

		
		// Set all tickers in open_position_map to false

		// if now >= next_minute:

			// Cancel all open buy orders

			// Set all tickers in buy_order_map to false

			// next_minute = now + 1000 (1min)