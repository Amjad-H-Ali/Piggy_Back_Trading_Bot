#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <json.hpp>
#include <time.h>

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

int main() {


#if defined (APIKEYID) &&  defined (APISECRETKEY)

    std::cout << "API-KEY-ID: " << STRINGIZE_VAL(APIKEYID) << std::endl;
    std::cout << "API-SECRET-KEY: " << STRINGIZE_VAL(APISECRETKEY) << std::endl;

	std::string response;


	// Number of days in each month: Unadjusted for leap-year.
	uint64_t months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


	// Previous date of initial day (1/1/2018)
	// Dec. 30 2017 and Dec. 31 2017 both fell on a weekend
	// So make Dec. 29 2017 the previous date.
	uint64_t prev_month = 11; // December

	uint64_t prev_day = 29; // Day 29

	uint64_t prev_year = 2017; // Year 2017

	// For all stocks, number of 10%-15% spikes that occurred and climbed another 15%, or more, after that. 
	uint64_t numerator = 0;

	// For all stocks, number of 10%-15% spikes that occurred.
	uint64_t denominator = 0;


	// Years range from [2018, 2021)
	for(uint64_t year = 2020; year < 2021; ++year) {

		// If leap-year, adjust February accordingly.
		if(year%4 == 0) months[1] = 29;

		else months[1] = 28;

		// Months range from [0,11]
		for(uint64_t  month = 0; month < 12; ++month) {

	

std::cout << "====================================================================== START OF NEW MONTH ======================================================================" << std::endl << std::endl;




			// Days range from [1,30], [1,31], [1,28], or [1,29] depending on month and if leap-year or not.
			for(uint64_t day = 1, days_in_month = months[month]; day <= days_in_month; ++day) {

				std::string response;

				// Request the Previous day closing price
				std::string request =	 "https://api.polygon.io/v1/open-close/AAPL/"
								
									+ 	 std::to_string(prev_year) + "-" + ( ((prev_month+1)<10) ? ("0"+std::to_string(prev_month+1)) : std::to_string(prev_month+1) ) + "-" 
							
									+ 	( (prev_day<10) ? ("0"+std::to_string(prev_day)) : std::to_string(prev_day) ) + "?apiKey=" STRINGIZE_VAL(APIKEYID);

std::cout << "Making request for previous day closing price: \n" << request << std::endl;

				fulfill_request(request, response);

				auto market_data_json = json::parse(response);

				// Today's date or future date: Cannot get day's closing price.
				if(market_data_json["status"] == "ERROR") break;

				// Previous day closing price
				float prev_day_closing_price = market_data_json["close"];


std::cout << "For the previous date of: " << prev_month+1 << "-" << prev_day << "-" << prev_year << ", \nthe closing price was: " << prev_day_closing_price << std::endl;

				// Test if market was open on this day (ie. closed because of national emergency or holiday ... etc.)
				request = 	"https://api.polygon.io/v2/aggs/ticker/AAPL/range/1/minute/" 

						+  	std::to_string(get_time_in_ms(month, day, year, 8, 30, 0)) + "/" + std::to_string(get_time_in_ms(month, day, year, 11, 30, 0)) 

						+ 	"?sort=asc&limit=50000&apiKey=" STRINGIZE_VAL(APIKEYID); 

std::cout << "Testing if market was open on: " << month+1 << "-" << day << "-" << year << std::endl;

				fulfill_request(request, response);

				market_data_json = json::parse(response);
std::cout << "Market was open? " << ((market_data_json["resultsCount"]==0) ? "False" : "True") << std::endl;
				// Market was closed on this day, so continue to next day.
				// Or this day was a short day. We don't trade on short days.
				if(market_data_json["resultsCount"] < 180) continue;


				// Opening prices of each minute of the stock from 8:30 AM (CST) - 11:30 AM (CST) on given day
				float open_prices[180] = {};

				// Responses containing minute charts. Entire array contains minute charts from 8:30 AM (CST) - 11:30 AM (CST) of given day
				std::string responses[6] = {};	

				// Trading hours we are interested in.
				uint64_t hours[4] = {8, 9, 10, 11};	


				// Populate responses array with the minute charts from 8:30 AM (CST) - 11:30 AM (CST).
				for(uint64_t i = 0; i < 3; ++i) {
						
std::cout << "For the date: " << month+1 << "-" << day << "-" << year << ", \n getting minute data between " << hours[i] << ":30:00 and " << hours[i+1] << ":00:00" << std::endl;

					request = 	"https://api.polygon.io/v2/aggs/ticker/AAPL/range/1/minute/" 

										+  	std::to_string(get_time_in_ms(month, day, year, hours[i], 30, 0)) + "/" + std::to_string(get_time_in_ms(month, day, year, hours[i+1], 00, 0)) 
					
										+ 	"?unadjusted=true&sort=asc&limit=50000&apiKey=" STRINGIZE_VAL(APIKEYID); 

					fulfill_request(request, responses[i<<1]);
std::cout << "Putting that data at index: " << (i<<1) << " of the responses array" << std::endl; 

std::cout << "For the date: " << month+1 << "-" << day << "-" << year << ", \n getting minute data between " << hours[i+1] << ":00:00 and " << hours[i+1] << ":30:00" << std::endl;

					request = 	"https://api.polygon.io/v2/aggs/ticker/AAPL/range/1/minute/" 

										+  	std::to_string(get_time_in_ms(month, day, year, hours[i+1], 00, 0)) + "/" + std::to_string(get_time_in_ms(month, day, year, hours[i+1], 30, 0)) 
					
										+ 	"?unadjusted=true&sort=asc&limit=50000&apiKey=" STRINGIZE_VAL(APIKEYID); 

					fulfill_request(request, responses[(i<<1)+1]);

std::cout << "Putting that data at index: " << ((i<<1)+1) << " of the responses array" << std::endl; 

				}

std::cout << "Populating the opening prices array" << std::endl;
				// Populate open_prices array with opening prices of stock for each minute between 8:30 AM (CST) - 11:30 AM (CST)
				for(uint64_t i = 0; i < 30; ++i) {
					
					market_data_json = json::parse(responses[0]);

					open_prices[i] = market_data_json["results"][i]["o"];

					market_data_json = json::parse(responses[1]);

					open_prices[i+30] = market_data_json["results"][i]["o"];

					market_data_json = json::parse(responses[2]);

					open_prices[i+60] = market_data_json["results"][i]["o"];

					market_data_json = json::parse(responses[3]);

					open_prices[i+90] = market_data_json["results"][i]["o"];										

					market_data_json = json::parse(responses[4]);

					open_prices[i+120] = market_data_json["results"][i]["o"];

					market_data_json = json::parse(responses[5]);

					open_prices[i+150] = market_data_json["results"][i]["o"];
				}																											


				// The stock movement meets all the conditions to qualify as one that spike 15% or more after already climbing 10%-15%.
				bool qualifies = false;
				// If the stock did spike, this is the price of it.
				float price_of_spike;

				// Percentage change of current open price and previous day's closing price.
				float percent_diff;
std::cout << "Looking for a spike" << std::endl;
				// For each minute between 8:30 AM (CST) - 11:30 AM (CST), look for a spike between 10%-15%
				for(uint64_t i = 0; i < 180; ++i) {

std::cout << "For time-stamp: " << (static_cast<unsigned>(i+1)/60)+8 << ":" <<   (i%60) << ":00" << "\n" << " the opening price is: " << open_prices[i] << std::endl;
					// Compute percentage change between current open price and previous day's closing price.
					percent_diff = ((open_prices[i] - prev_day_closing_price)/prev_day_closing_price)*100;

std::cout << "The percent change from previous closing day is: " << percent_diff << "%" << std::endl;
					// Stock spiked between 10%-15%
					if( (percent_diff >= 10) & (percent_diff <= 15) ) {

						price_of_spike = open_prices[i];

std::cout << "We found a spike at price: " << price_of_spike << std::endl;

						++denominator;

						++i; // Go to open price immediately after spike.
std::cout << "Looking for further spike" << std::endl;
						// If spike was found, search further (after spike) and ensure stock price did not fall below previous day's closing price, and
						// search for further spike of 15% or more.
						for(; i < 180; ++i) {

						
							// After spike, stock price fell below previous day's closing price, so break: Does not qualify.
							if(open_prices[i] <= prev_day_closing_price) {

std::cout << "It looks like the stock price fell below the previous day's closing price of: " << prev_day_closing_price << "\n" << " and hit a price of: " << open_prices[i] << std::endl;
								break;

							}
							
							// Compute percent change between current open price and initial spike.
							float further_climb = ((open_prices[i] - price_of_spike)/price_of_spike)*100;
std::cout << "Percent change from last spike: " << further_climb << " at a price of: " << open_prices[i] << std::endl;

std::cout << "Price that immediately follows: " << ((i+1 < 180) ? std::to_string(open_prices[i+1]) : "this was the last price") << std::endl; 
							// If stock climbed further, and open price of next minute was higher, then this stock qualifies 
							// as one that spiked after the initial spike.
							if( (further_climb >= 15) && (i+1 < 180) && (open_prices[i+1] > open_prices[i])) {

std::cout << "Stock climbed another 15% or more and hit a price of: " << open_prices[i] << std::endl; 
								++numerator;

								qualifies = true;

								break;

							}

						}

						// If the stock qualifies as one that we will invest in, then break and move on to the next day.
						if(qualifies) break;
						
					}
				}
std::cout << "Switching previous date to: " <<  month+1 << "-" << day << "-" << year << std::endl;		

				// Set previous date before going on to next day.
				prev_month = month;

				prev_day = day;

				prev_year = year;

// std::cout << "Computing probability: " << ((denominator!=0) ? std::to_string(numerator/denominator) : "No 10%-15% spike found.") << std::endl;

				// For now, work with one day
				// return 0;

			}
		}
	}
std::cout << "Numerator (Number of times stock spiked 15% or more further after already spiking 10%-15%: " << numerator << std::endl; 
std::cout << "Denominator (Number of times stock spiked between 10%-15%): " << denominator << std::endl;
std::cout << "Computing probability: " << ((denominator!=0) ? std::to_string(numerator/denominator) : "No 10%-15% spike found.") << std::endl;

	// curl_easy_cleanup(curl);
	
    return 0;

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif
}