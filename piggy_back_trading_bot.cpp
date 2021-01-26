#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <json.hpp>
#include <time.h>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <chrono>
#include <thread>
#include <mutex>
#include <cpprest/ws_client.h>


using json = nlohmann::json;
using namespace web;
using namespace web::websockets::client;

#define STR(X) #X
#define STRINGIZE_VAL(X) STR(X)
#define SLEEP(MS) std::this_thread::sleep_for(std::chrono::milliseconds(MS))
#define PREV_MONTH_S "01"
#define PREV_DAY_S 	"20"
#define PREV_YEAR_S  "2021"
#define MONTH_S "01"
#define DAY_S 	"21"
#define YEAR_S  "2021"
#define NEXT_MONTH_S "01"
#define NEXT_DAY_S 	"22"
#define NEXT_YEAR_S  "2021"
#define PREV_MONTH_I 0
#define PREV_DAY_I 	20
#define PREV_YEAR_I  2021
#define MONTH_I 0
#define DAY_I 	21
#define YEAR_I  2021
#define NEXT_MONTH_I 0
#define NEXT_DAY_I 	22
#define NEXT_YEAR_I  2021
#define NOW (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
#define HOUR 8
#define MIN 30
#define SEC 5
#define NEXT_HOUR 8
#define NEXT_MINUTE 31
#define NEXT_SEC 5
#define ACCOUNT "paper-api"






// Readable time, used for debugging
void current_human_time() {

   time_t t; // t passed as argument in function time()
   struct tm * tt; // decalring variable for localtime()
   time (&t); //passing argument to time()
   tt = localtime(&t);
   std::cout << "Current Day, Date and Time is = "<< asctime(tt) << std::endl;

}

// Appends response to given output string
static size_t write_data(const char* in, std::size_t size, std::size_t num, char* out) {

	// Append response to output string.
	((std::string*)out)->append(in, size*num);

	return size * num;
}

// Given a string request and parameters does an HTTP POST request to a server and stores
// response in given string response object.
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

// Given a string request and parameters does an HTTP DELETE request to a server and stores
// response in given string response object.
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


// Given a string request, does an HTTP GET request to a server and stores
// response in given string response object.
uint64_t fulfill_get_request(const std::string& request, std::string& response, const std::string& parameters = "") {

	// Set HTTP header
	static CURL *curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, (request+parameters).c_str());
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





void print_account() {

	uint64_t status_code;

	std::string response;

	decltype(json::parse(response)) market_data_json;

	// Request account information
	std::string request = "https://" ACCOUNT ".alpaca.markets/v2/account";
	
	while( (status_code = fulfill_get_request(request, response)) != 200 ) {

		std::cerr << "Error while requesting account. Status Code: " << status_code << " . Retrying ..." << std::endl;

		SLEEP(500);
	}


	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Error while parsing account" << std::endl;

		}
		catch(nlohmann::detail::type_error err) {

			std::cerr << "Type error while parsing account" << std::endl;

		}
	}

	std::cout << "ACCOUNT: " << market_data_json.dump() << std::endl;

}

void cancel_unfulfilled_buy_orders() {

	std::cout << "System is shutting down..." << std::endl;

	std::string request, response;
	uint64_t status_code;

	decltype(json::parse(response)) market_data_json;

	// Get a list of all unfulfilled orders
	request = "https://" ACCOUNT ".alpaca.markets/v2/orders";

	while((status_code = fulfill_get_request(request, response)) != 200) {

		std::cerr << "Error while requesting list of orders. Status code: " << status_code << " . Retrying ..." << std::endl;

		SLEEP(500);
	}
	while(true) {

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

	// Cancel each unfulfilled buy order
	request = "https://" ACCOUNT ".alpaca.markets/v2/orders";
	
	uint64_t order_indx = 0;
	while(market_data_json[order_indx] != nullptr) {

std::cout << "17" << std::endl;
		// Order type is buy, delete it
		if(static_cast<std::string>(market_data_json[order_indx]["side"]) == static_cast<std::string>("buy")) {
			std::string order_id = "/" + static_cast<std::string>(market_data_json[order_indx]["id"]);


			while((status_code = fulfill_delete_request(request, response, order_id)) == 429) {

				std::cerr << "Error while requesting to cancel order. Status code: " << status_code << " . Retrying ..." << std::endl;

				SLEEP(500);
			}


			if(status_code != 204) {

				std::cerr << "Error while requesting to cancel order. Status code: " << status_code << std::endl;
				std::cerr << "Order: " << market_data_json[order_indx].dump() << std::endl;
			}

		} // If statement: Delete only buy orders


std::cout << "18" << std::endl;

		++order_indx;

	} // While loop: Delete buy orders
}


void get_open_sell_orders(std::map<std::string, bool>& map_to_sell_orders) {

	std::string request, response;
	uint64_t status_code;

	decltype(json::parse(response)) market_data_json;

	// Get a list of all unfulfilled orders
	request = "https://" ACCOUNT ".alpaca.markets/v2/orders";

	while((status_code = fulfill_get_request(request, response)) != 200) {

		std::cerr << "Error while requesting list of orders. Status code: " << status_code << " . Retrying ..." << std::endl;

		SLEEP(500);
	}
	while(true) {

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



	uint64_t order_indx = 0;
	while(market_data_json[order_indx] != nullptr) {

		// Order type is sell, insert in map
		if(static_cast<std::string>(market_data_json[order_indx]["side"]) == static_cast<std::string>("sell")) {
			map_to_sell_orders.emplace(market_data_json[order_indx]["symbol"], true);
		}

		++order_indx;

	}
}


void liquidate_positions(const std::map<std::string, bool> &map_with_sell_orders) {

	std::string request, response;
	uint64_t status_code;

	decltype(json::parse(response)) market_data_json;

	// Request current live positions
	request = "https://" ACCOUNT ".alpaca.markets/v2/positions";
	
	
	while( (status_code = fulfill_get_request(request, response)) != 200 ) {

		std::cerr << "Error while requesting positions. Status Code: " << status_code << " . Retrying ..." << std::endl;

		SLEEP(500);
	}


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

	auto it_end = map_with_sell_orders.end();

	// For each position, submit a sell order. 
	uint64_t position_indx = 0;
	while(market_data_json[position_indx] != nullptr) {

		auto it = map_with_sell_orders.find(static_cast<std::string>(market_data_json[position_indx]["symbol"]));

		// Position does not have a sell order in place, so liquidate it.
		if((it == it_end) || (it->second == false)) {


			std::string sym = "/" + static_cast<std::string>(market_data_json[position_indx]["symbol"]);


			while((status_code = fulfill_delete_request(request, response, sym)) == 429) {

				std::cerr << "Error while requesting to cancel position. Status code: " << status_code << " . Retrying ..." << std::endl;

				SLEEP(500);
			}


			if(status_code != 204) {

				std::cerr << "Error while requesting to cancel position. Status code: " << status_code << std::endl;
				std::cerr << "Position: " << market_data_json[position_indx].dump() << std::endl;
			}
		}


		++position_indx;

	}
}

float get_cash() {

	std::string request, response;
	uint64_t status_code;

	decltype(json::parse(response)) market_data_json;

	// Request account information
	request = "https://" ACCOUNT ".alpaca.markets/v2/account";
	
	while( (status_code = fulfill_get_request(request, response)) != 200 ) {

		std::cerr << "Error while requesting account in get_cash function. Status Code: " << status_code << " . Retrying ..." << std::endl;

		SLEEP(500);
	}


	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Error while parsing account in get_cash function" << std::endl;

		}
		catch(nlohmann::detail::type_error err) {

			std::cerr << "Type error while parsing account in get_cash function" << std::endl;

		}
	}

	return std::stof(static_cast<std::string>(market_data_json["cash"]));
}



uint64_t get_capital_distribution(const std::vector<std::string>& sell_order_ids) {

	uint64_t capital = 0;

	std::string request, response;
	uint64_t status_code;

	decltype(json::parse(response)) market_data_json;

	// Request current live positions
	request = "https://" ACCOUNT ".alpaca.markets/v2/orders/";

	for(const std::string& sell_order_id : sell_order_ids) {

		while((status_code = fulfill_get_request(request, response, sell_order_id)) != 200) {

			std::cerr << "Error while requesting sell order info in get_capital_distribution function. Status code: " << status_code << " . Retrying ..." << std::endl;

			SLEEP(500);
		}

		// Attempt to parse response into json
		while(true) {

			try {

				market_data_json = json::parse(response);

				break;
			}
			catch(nlohmann::detail::parse_error err) {

				std::cerr << "Error while parsing sell order info in get_capital_distribution function" << std::endl;

			}
			catch(nlohmann::detail::type_error err) {

				std::cerr << "Type error while parsing sell order info in get_capital_distribution function" << std::endl;

			}
		}

		uint64_t filled_qty = std::stoull(static_cast<std::string>(market_data_json["filled_qty"]));
		uint64_t filled_avg_price = std::stof(static_cast<std::string>(market_data_json["filled_avg_price"]));

		capital += (filled_avg_price*filled_qty);
		
	}

	return capital;
	
}

int main(void) {

/* Preparation */	

#if defined (APIKEYID) &&  defined (APISECRETKEY)

	std::mutex m1;
	std::string response;
	std::map<std::string, std::vector<std::string>> capital_distribution_map;

	// Request market data for previous open day
	std::string request = "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/" PREV_YEAR_S "-" PREV_MONTH_S "-" PREV_DAY_S "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);
	
	decltype(json::parse(response)) market_data_json;

	fulfill_get_request(request, response);

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
	} // For loop: previous market data map

std::cout << "02" << std::endl;

	// Put thread to sleep until next market open
	uint64_t ms_till_open = get_time_in_ms(MONTH_I, DAY_I, YEAR_I, HOUR, MIN, SEC) - NOW;

	SLEEP(ms_till_open);

	// Liquidate any open positions from previous market day : Rare
	request = "https://" ACCOUNT ".alpaca.markets/v2/positions";
	fulfill_delete_request(request, response, "");

	// Request current market data
	request = "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/" YEAR_S "-" MONTH_S "-" DAY_S "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

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

	// Ticker and VWAP
	std::map<std::string, std::pair<float, float>> watchlist;

	std::map<std::string, Prev_Market_Data_T>::iterator it_end = prev_market_data_map.end();
	// Put all stocks in a watchlist that meet the requirement below:
	// If stock has a relative volume (from last trading day) spike of 500% or more, and has a spike in price of 5% or more, then trade stock
	// when its price reaches a price 2% (or more) below the VWAP. Sell the stock if the price declines 0.5% or more.
	for(uint64_t stock_indx = 0, stock_count = market_data_json["resultsCount"]; stock_indx < stock_count; ++stock_indx) {

		std::map<std::string, Prev_Market_Data_T>::iterator it_prev_market = prev_market_data_map.find(static_cast<std::string>(market_data_json["results"][stock_indx]["T"]));

		// Previous data not found
		if(it_prev_market == it_end) continue;

		// Compute percent change from previous day
		float price_percent_change =  (static_cast<float>(market_data_json["results"][stock_indx]["o"]) - (it_prev_market->second).price)/(it_prev_market->second).price,

   			  volume_percent_change = (static_cast<float>(market_data_json["results"][stock_indx]["v"]) - (it_prev_market->second).volume)/(it_prev_market->second).volume;

		// Stock's price spiked 5% or more, volume spiked 500% or more, total day's volume of last trading day was 150k or more.
		if((price_percent_change >= 0.05) && (volume_percent_change >= 5) && ((it_prev_market->second).volume >= 150000)) {

			std::string ticker = static_cast<std::string>(market_data_json["results"][stock_indx]["T"]);

			watchlist.emplace(ticker, std::pair<float, float>(-1, -1));

			capital_distribution_map.emplace(ticker, std::vector<std::string>());
		}
	} // For loop: creating watchlist


	std::cout << "Watchlist Size: " << watchlist.size() << std::endl;

	// If no stocks meet the requirement today, then pause algo till next market open
	if (watchlist.size() == 0) {

		int64_t ms_till_tomorrow = get_time_in_ms(NEXT_MONTH_I, NEXT_DAY_I, NEXT_YEAR_I, HOUR, MIN, SEC) - NOW;

		SLEEP(ms_till_tomorrow);
	}
	std::cout << "06" << std::endl;


	float cash = get_cash();

	// Evenly allocate cash to all stocks in watchlist. Cash distribution should be no greater than 5% of the total cash
	uint64_t capital_distribution = std::min(cash/watchlist.size(), cash*static_cast<float>(0.05));

	std::cout << "CASH D: " << capital_distribution << std::endl;
	
	std::map<std::string, bool> open_position_map, buy_order_map;
	std::map<std::string, uint64_t> sell_order_map;

	// Prepare string of watchlist tickers for market data request and initialize maps
	std::string watchlist_tickers = "";
	for(const auto& ticker_pair : watchlist) {

		watchlist_tickers += ("A." + ticker_pair.first + ",");

		open_position_map[ticker_pair.first] = false, buy_order_map[ticker_pair.first] = false, sell_order_map[ticker_pair.first] = 0;
	}

	// Delete last comma
	watchlist_tickers.erase(watchlist_tickers.end()-1);

	
std::cout << "07" << std::endl;


std::cout << "08" << std::endl;

/* End of Preparation */	


/* Subscribe */

	websocket_callback_client client;
	websocket_outgoing_message out_msg;

	decltype(json::parse(response)) market_data_json_real_time;

	// Handler for incoming messages from server
	client.set_message_handler([&](const websocket_incoming_message &in_msg)
	{
		(in_msg.extract_string()).then([&](const std::string& body) {

			try {
				m1.lock();
				market_data_json_real_time = json::parse(body);
				

				uint64_t real_time_indx = 0;
				// Update VWAP in map
				while(market_data_json_real_time[real_time_indx] != nullptr) {
					std::string event = market_data_json_real_time[real_time_indx]["ev"];
					if(event == "A") {
						std::cout << "Ticker: " << market_data_json_real_time[real_time_indx]["sym"] << std::endl;
						std::cout << "VWAP: " << market_data_json_real_time[real_time_indx]["vw"] << std::endl;
						std::cout << "Start Time-Stamp: " << market_data_json_real_time[real_time_indx]["s"] << std::endl;
						std::cout << "End Time-Stamp: " << market_data_json_real_time[real_time_indx]["e"] << std::endl;


						current_human_time();

						std::string ticker = market_data_json_real_time[real_time_indx]["sym"];

						watchlist[ticker].first = market_data_json_real_time[real_time_indx]["vw"];
						watchlist[ticker].second = market_data_json_real_time[real_time_indx]["c"];

					}
					++real_time_indx;
				}
				m1.unlock();

				// if(market_data_json_real_time)
				
			}
			catch(const nlohmann::detail::parse_error &err) {

				m1.unlock();

				std::cout << "No real-time data to Parse" << std::endl;
				
			}
			
			
		}).wait();;
	});
	

	// Connect to live stream
  	client.connect("wss://socket.polygon.io/stocks").wait();
	
		

	// Send authentication
	out_msg.set_utf8_message("{\"action\":\"auth\",\"params\":\"" STRINGIZE_VAL(APIKEYID) "\"}");
	client.send(out_msg).wait();

		
	// Subscribe to stocks
	out_msg.set_utf8_message("{\"action\":\"subscribe\",\"params\":\"" + watchlist_tickers + "\"}");
	std::cout << "{\"action\":\"subscribe\",\"params\":\"" + watchlist_tickers + "\"}" << std::endl;
	client.send(out_msg).wait();

	
/* End of Subscribe */

/* Initial Buy Orders */	


	while(true) {

		m1.lock();
		if((market_data_json_real_time != nullptr) && (static_cast<std::string>(market_data_json_real_time[0]["ev"]) == "A" )) {
			m1.unlock();
			break;
		}
		m1.unlock();
	}

	
	// Submit limit order for each stock in watchlist
	for(const auto& ticker_pair : buy_order_map) {

		std::cout << "09" << std::endl;

		std::string ticker = ticker_pair.first;

		m1.lock();
		if(watchlist[ticker].first == -1) {
			m1.unlock();
			continue;
		}
		m1.unlock();

		m1.lock();
		std::pair<float, float>& vwap_close_pair = watchlist[ticker];

		float limit_price;
		// If close price is less then or equal to vwap
		if(vwap_close_pair.second <= vwap_close_pair.first) {
			// Compute 2% below price (closing price of last second)
			limit_price = vwap_close_pair.second*static_cast<float>(0.98);
		}
		else {

			// Compute 2% below VWAP
			limit_price = vwap_close_pair.first*static_cast<float>(0.98);
		}

		m1.unlock();
		// Compute quantity to buy
		uint64_t qty = capital_distribution/limit_price;

		request = "https://" ACCOUNT ".alpaca.markets/v2/orders";

		// Create order string
		decltype(json::parse(response)) param_json;
	std::cout << "12" << std::endl;

		param_json["side"]        		= "buy";
		param_json["symbol"]      		= ticker;
		param_json["type"]   	  		= "limit";
		param_json["limit_price"] 		= limit_price;
		param_json["qty"]         		= qty;
		param_json["time_in_force"]     = "gtc";



	std::cout << "13" << std::endl;
		uint64_t status_code;
		// Submit order. Decrease quantity and resubmit buy order if insufficient quantity 
		while((status_code = fulfill_post_request(request, response, param_json.dump())) != 200) {

			std::cerr << "Error while submitting reentry order for " << ticker << ". Status Code: " << status_code << " . Retrying ..." << std::endl;

			if(status_code == 403) {

				std::cerr << "Insuffucient Buying Power. Decreasing qty" << std::endl;
				m1.lock();

				std::pair<float, float>& vwap_close_pair = watchlist[ticker];
				// If close price is less then or equal to vwap
				if(vwap_close_pair.second <= vwap_close_pair.first) {
					// Compute 2% below price (closing price of last second)
					limit_price = vwap_close_pair.second*static_cast<float>(0.98);
				}
				else {

					// Compute 2% below VWAP
					limit_price = vwap_close_pair.first*static_cast<float>(0.98);
				}
				m1.unlock();

				// Compute quantity to buy
				qty = capital_distribution/limit_price;
				param_json["qty"] = (qty/=2);
			}
			else {
				SLEEP(500);
			}
		}

		std::cout << "Init buy order: " << param_json["symbol"] << "  " << param_json["limit_price"] << "  " << param_json["qty"] << std::endl;
		current_human_time();
		print_account();

		buy_order_map[ticker]  = true;

	} // While-loop: Initial limit buy orders of all stocks in watchlist

/* End of Initial Buy Orders */

/* Sell Orders/Reentry Orders/Cancel Sold Orders/Refresh Orders */

	// For each position, submit a sell order. For sold positions, submit new limit buy order. 
	// Every minute, refresh unfulfilled orders with updated limit entry prices.
	uint64_t end_time_ms = get_time_in_ms(MONTH_I, DAY_I, YEAR_I, 9, 0, 0);
	uint64_t last_minute = NOW;
	while(static_cast<uint64_t>(NOW) <  end_time_ms) {
		
		// Request current live positions
		request = "https://" ACCOUNT ".alpaca.markets/v2/positions";
		uint64_t status_code;
		
		while( (status_code = fulfill_get_request(request, response)) != 200 ) {

			std::cerr << "Error while requesting positions. Status Code: " << status_code << " . Retrying ..." << std::endl;

			SLEEP(500);
		}


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


/* Sell Orders */

		// For each position, submit a sell order. 
		uint64_t position_indx = 0;
		while(market_data_json[position_indx] != nullptr) {

std::cout << "15" << std::endl;

			std::string ticker = market_data_json[position_indx]["symbol"];

			uint64_t qty_of_position = std::stoull(static_cast<std::string>(market_data_json[position_indx]["qty"])),
					 qty_sold        = sell_order_map[ticker];
			
			buy_order_map[ticker]     = false;
			open_position_map[ticker] = true;
	

			// Submit sell order if haven't already submitted, or if portion remains unsold
			if(qty_of_position > qty_sold) {

		
				request = "https://" ACCOUNT ".alpaca.markets/v2/orders";

			
				decltype(json::parse(response)) param_json;

				param_json["side"]        		= "sell";
				param_json["symbol"]      		= ticker;
				param_json["type"]   	  		= "trailing_stop";
				param_json["trail_percent"]     = "0.5";
				param_json["qty"]         		= (qty_of_position - qty_sold);
				param_json["time_in_force"]     = "gtc";

			
				uint64_t status_code; 
				decltype(json::parse(response)) sell_order_response_json;
		
				while((status_code = fulfill_post_request(request, response, param_json.dump())) != 200) {

					std::cerr << "Error while submitting sell order for " << ticker << ". Status Code: " << status_code << " . Retrying ..." << std::endl;

					SLEEP(500);
				}


				// Attempt to parse response into json
				while(true) {

					try {

						sell_order_response_json = json::parse(response);

						break;
					}
					catch(nlohmann::detail::parse_error err) {

						std::cerr << "Error while parsing sell order response" << std::endl;

					}
					catch(nlohmann::detail::type_error err) {

						std::cerr << "Type error while parsing sell order response" << std::endl;

					}
				}

				std::string sell_order_id = sell_order_response_json["id"];

				capital_distribution_map[ticker].emplace_back(sell_order_id);

				std::cout << "Sell order: "  << param_json["symbol"] << "  " << param_json["qty"]  << std::endl;
				current_human_time();
				print_account();

std::cout << "16" << std::endl;

				sell_order_map[ticker] = qty_of_position;
				
			} // If statement: Sell Order

			
			++position_indx;

		} // While loop: Submit sell orders 

/* End of Sell Orders */	


/* Reentry Orders */

		// If position was sold, submit new limit buy order.
		// Prepare string with stocks to rebuy for market data request
		std::vector<std::string> reentry_watchlist_tickers;
		for(const auto& open_position_pair : open_position_map) {

			if((buy_order_map[open_position_pair.first] == false) && (open_position_pair.second == false)) {

				reentry_watchlist_tickers.emplace_back(open_position_pair.first);
			}
		} // For loop: Prepare string with stocks to rebuy 

		uint64_t reentry_count = reentry_watchlist_tickers.size();

		// If there are stocks to rebuy, submit limit order for each one
		if(reentry_count > 0) {

			std::cout << "sz: " << reentry_watchlist_tickers.size() << std::endl;
			for(const std::string& ticker : reentry_watchlist_tickers) {
				
				std::vector<std::string>& sell_order_ids = capital_distribution_map[ticker];
				uint64_t capital_redistribution = get_capital_distribution(sell_order_ids);
				
				sell_order_ids.clear();

				m1.lock();
				if(watchlist[ticker].first == -1) {
					m1.unlock();
					continue;
				}
				m1.unlock();
				

				m1.lock();
				std::pair<float, float>& vwap_close_pair = watchlist[ticker];

				float limit_price;
				// If close price is less then or equal to vwap
				if(vwap_close_pair.second <= vwap_close_pair.first) {
					// Compute 2% below price (closing price of last second)
					limit_price = vwap_close_pair.second*static_cast<float>(0.98);
				}
				else {

					// Compute 2% below VWAP
					limit_price = vwap_close_pair.first*static_cast<float>(0.98);
				}
				m1.unlock();

				// Calculate the amount to buy
				uint64_t qty = capital_redistribution/limit_price;

				// Create order string and submit request
				request = "https://" ACCOUNT ".alpaca.markets/v2/orders";

				decltype(json::parse(response)) param_json;

std::cout << "12" << std::endl;

				param_json["side"]        		= "buy";
				param_json["symbol"]      		= ticker;
				param_json["type"]   	  		= "limit";
				param_json["limit_price"] 		= limit_price;
				param_json["qty"]         		= qty;
				param_json["time_in_force"]     = "gtc";

std::cout << "13" << std::endl;
				// If buy order failed, resubmit order with less quantity
				uint64_t status_code; 
				while((status_code = fulfill_post_request(request, response, param_json.dump())) != 200) {

					std::cerr << "Error while submitting reentry order for " << ticker << ". Status Code: " << status_code << " . Retrying ..." << std::endl;

					if(status_code == 403) {

						std::cerr << "Insuffucient Buying Power. Decreasing qty" << std::endl;

						m1.lock();
						std::pair<float, float>& vwap_close_pair = watchlist[ticker];

						// If close price is less then or equal to vwap
						if(vwap_close_pair.second <= vwap_close_pair.first) {
							// Compute 2% below price (closing price of last second)
							limit_price = vwap_close_pair.second*static_cast<float>(0.98);
						}
						else {

							// Compute 2% below VWAP
							limit_price = vwap_close_pair.first*static_cast<float>(0.98);
						}
						m1.unlock();

						// Calculate the amount to buy
						qty = capital_redistribution/limit_price;

						param_json["qty"] = (qty/=2);
					}
					else {
						SLEEP(500);
					}
				}

				std::cout << "Rebuy order: : " << param_json["symbol"] << "  " << param_json["limit_price"] << "  " << param_json["qty"] << std::endl;
				current_human_time();
				print_account();

				sell_order_map[ticker]    = 0;
				buy_order_map[ticker]     = true;
				open_position_map[ticker] = false;

			} // For loop: Calculate limit price and submit order for each stock to rebuy


		} // If statement: If there are stocks to rebuy, submit limit order for each one

/* End of Reentry Orders */


/* Refresh Orders */

		// New minute, cancel all unfulfilled buy orders
		if((NOW - last_minute) >= 60000) {

			// Get a list of all unfulfilled orders
			request = "https://" ACCOUNT ".alpaca.markets/v2/orders";

			uint64_t status_code; 

			while((status_code = fulfill_get_request(request, response)) != 200) {

				std::cerr << "Error while requesting list of orders. Status code: " << status_code << " . Retrying ..." << std::endl;

				SLEEP(500);
			}
			while(true) {

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

			// Cancel each unfulfilled buy order
			request = "https://" ACCOUNT ".alpaca.markets/v2/orders";
	
			uint64_t order_indx = 0;
			while(market_data_json[order_indx] != nullptr) {

std::cout << "17" << std::endl;
				// Order type is buy, delete it
				if(static_cast<std::string>(market_data_json[order_indx]["side"]) == static_cast<std::string>("buy")) {
					std::string order_id = "/" + static_cast<std::string>(market_data_json[order_indx]["id"]);

	
					while((status_code = fulfill_delete_request(request, response, order_id)) == 429) {

						std::cerr << "Error while requesting to cancel order. Status code: " << status_code << " . Retrying ..." << std::endl;

						SLEEP(500);
					}


					if(status_code != 204) {

						std::cerr << "Error while requesting to cancel order. Status code: " << status_code << std::endl;
						std::cerr << "Order: " << market_data_json[order_indx].dump() << std::endl;
					}

				} // If statement: Delete only buy orders


std::cout << "18" << std::endl;

				++order_indx;

			} // While loop: Delete buy orders

			// Set all buy orders in map to false
			for(auto& buy_order_pair: buy_order_map) {
				buy_order_pair.second = false;
			}

			print_account();

			// Update minute
			last_minute = NOW;

		} // If Statement: New minute, cancel all unfulfilled buy orders

/* End of Refresh Orders */


		// Set all open positions to false
		for(auto& open_position_pair : open_position_map) {
			open_position_pair.second = false;
		}

		SLEEP(300);

	} // While loop: Submit sell orders, submit buy orders on stocks that were sold

/* End of Sell Orders/Reentry Orders/Cancel Sold Orders/Refresh Orders */



	// Cancel all lefover open buy orders
	cancel_unfulfilled_buy_orders();

	// Liquidate positions that don't already have sell orders in place.
	std::map<std::string, bool> map_to_sell_orders;
	get_open_sell_orders(map_to_sell_orders);
	liquidate_positions(map_to_sell_orders);


	client.close().wait();

	return 0;

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif

} // main





