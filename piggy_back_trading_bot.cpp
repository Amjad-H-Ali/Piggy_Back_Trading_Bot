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
#define START_DAY 	"04"
#define START_YEAR  "2021"

// Appends response to given output string
static size_t write_data(const char* in, std::size_t size, std::size_t num, char* out) {

	// Append response to output string.
	((std::string*)out)->append(in, size*num);

	return size * num;
}

// Given a string request, does an HTTP request to a server and stores
// response in given string response object.
uint64_t fulfill_request(const std::string& request, std::string& response) {

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

	fulfill_request(request, response);

	decltype(json::parse(response)) market_data_json;

	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Part of response lost in transmission" << std::endl;

		}
	}



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

	// Put thread to sleep until next market open
	uint64_t ms_till_open = get_time_in_ms(0, 6, 2021, 8, 30, 0) - (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

	std::this_thread::sleep_for(std::chrono::milliseconds(ms_till_open));

	// Request current market data
	request = "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/2021-01-06?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

	fulfill_request(request, response);

	// Attempt to parse response into json
	while(true) {

		try {

			market_data_json = json::parse(response);

			break;
		}
		catch(nlohmann::detail::parse_error err) {

			std::cerr << "Part of response lost in transmission" << std::endl;

		}
	}

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


		}
	}



#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif

}