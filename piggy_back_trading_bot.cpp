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


// Returns a new vector of stocks that are green for the given minute interval.
std::vector<std::string> filter_spike_tickers(const std::vector<std::string>& tickers, uint64_t ticker_indx, uint64_t  month, uint64_t day, uint64_t year) {

	std::vector<std::string> qualified_tickers;
	std::string request, response;



	for(const std::string& ticker : tickers) {

		request = 	"https://api.polygon.io/v2/aggs/ticker/" + ticker + "/range/1/minute/" 

				+  	std::to_string(get_time_in_ms(month, day, year, 8, 30, 0)) + "/" + std::to_string(get_time_in_ms(month, day, year, 8, 31 + ticker_indx, 0)) 
			
				+ 	"?unadjusted=false&sort=asc&limit=50000&apiKey=" STRINGIZE_VAL(APIKEYID); 

		fulfill_request(request, response);

		auto market_data_json = json::parse(response);

		try{
			if(market_data_json["results"][ticker_indx]["c"] > market_data_json["results"][ticker_indx]["o"]) {

				qualified_tickers.emplace_back(ticker);
			}
		}
		catch(nlohmann::detail::parse_error parse_error) {

			std::cerr << "JSON parse error in filter_spike_tickers function: " << std::endl;
		}
		catch(nlohmann::detail::type_error type_error) {

			std::cerr << "JSON type error in filter_spike_tickers function: " << std::endl;
		}
	}

	return qualified_tickers;
}



int main(void) {

#if defined (APIKEYID) &&  defined (APISECRETKEY)


	// std::cout << (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) << std::endl;

	// uint64_t t = get_time_in_ms(11, 29, 2020, 1, 35, 0) - (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

	// std::this_thread::sleep_for(std::chrono::milliseconds(t));


	// Request  stock data for previous open day
	std::string request =  "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/2020-12-24?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

	std::string response;

	fulfill_request(request, response);

	auto market_data_json = json::parse(response);

	std::map<std::string, float> prev_price_map;

	std::vector<std::string> spike_tickers;

	// Get all stocks closing prices.
	for(uint64_t ticker_indx = 0, ticker_count = market_data_json["resultsCount"]; ticker_indx < ticker_count; ++ticker_indx) {

		std::string t = market_data_json["results"][ticker_indx]["T"];

		if( (t.size() > 10) || (static_cast<uint64_t>(market_data_json["results"][ticker_indx]["v"]) < 400000) || (static_cast<float>(market_data_json["results"][ticker_indx]["o"]) < 1)) continue;

		prev_price_map.emplace(t, market_data_json["results"][ticker_indx]["c"]);

	}


	// Request stock data for today
	request = "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/2020-12-28?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

	fulfill_request(request, response);

	market_data_json = json::parse(response);


	// Search for all stocks that opened with a spike of at least 5 percent
	for(uint64_t ticker_indx = 0, ticker_count = market_data_json["resultsCount"]; ticker_indx < ticker_count; ++ticker_indx) {

		std::string t = market_data_json["results"][ticker_indx]["T"];

		std::map<std::string, float>::iterator it = prev_price_map.find(t);

		// Ticker does not have a previous day closing price; It must be a new ticker.
		if(it == prev_price_map.end()) continue;


		float percent_change = ((static_cast<float>(market_data_json["results"][ticker_indx]["o"]) - it->second)/it->second)*100;


		if(percent_change > 5) {

			spike_tickers.emplace_back(it->first);

		}

	}


	// Phase 1: Is the first minute interval green.
	std::vector<std::string> phase1_spike_tickers =  filter_spike_tickers(spike_tickers, 0, 12, 29, 2020);

	// Phase 2: Is the second minute interval green.
	std::vector<std::string> phase2_spike_tickers =  filter_spike_tickers(phase1_spike_tickers, 1, 12, 29, 2020);

	// Phase 3: Is the third minute interval green.
	std::vector<std::string> phase3_spike_tickers =  filter_spike_tickers(phase2_spike_tickers, 2, 12, 29, 2020);





#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif

}