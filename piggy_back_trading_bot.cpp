#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <json.hpp>
#include <time.h>
#include <vector>
#include <map>
#include <algorithm>

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

	uint64_t prev_day = 31; // Day 29

	uint64_t prev_year = 2019; // Year 2017

	// For all stocks, number of 10%-15% spikes that occurred and climbed another 15%, or more, after that. 
	uint64_t numerator = 0;

	// For all stocks, number of 10%-15% spikes that occurred.
	uint64_t denominator = 0;

	float gains = 0;

	float losses = 0;

	uint64_t count = 0;

	uint64_t errors = 0;

	float real_gains_m = 50000;

	std::vector<float> pl_percentages[2500];
	uint64_t pl_indx = 0;

	// Years range from [2018, 2021)
	for(uint64_t year = 2020; year < 2021; ++year) {

		std::vector<float> pl_percentages_y[365];
		uint64_t pl_indx_y = 0;

		// If leap-year, adjust February accordingly.
		if(year%4 == 0) months[1] = 29;

		else months[1] = 28;

		// Months range from [0,11]
		for(uint64_t  month = 0; month < 12; ++month) {

			std::vector<float> pl_percentages_m[35];
			uint64_t pl_indx_m = 0;



			// Days range from [1,30], [1,31], [1,28], or [1,29] depending on month and if leap-year or not.
			for(uint64_t day = 1, days_in_month = months[month]; day <= days_in_month; ++day, ++pl_indx, ++pl_indx_y, ++pl_indx_m) {

std::cout << month+1 << "/" << day << "/" << year << std::endl;				

				std::string response;

				std::map<std::string, float> prev_day_closing_price_map;

				// Request the Previous day's closing price for all stocks
				std::string request =	 "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/"
								
									+ 	 std::to_string(prev_year) + "-" + ( ((prev_month+1)<10) ? ("0"+std::to_string(prev_month+1)) : std::to_string(prev_month+1) ) + "-" 
							
									+ 	( (prev_day<10) ? ("0"+std::to_string(prev_day)) : std::to_string(prev_day) )

									+ "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

// std::cout << "Making request for previous day closing price for all stocks: \n" << request << std::endl;

				fulfill_request(request, response);

				auto market_data_json = json::parse(response);

				// Either today's date, future date, or market was closed: Cannot get day's closing price.
				if(market_data_json["resultsCount"] == 0) continue;

// std::cout << "Inserting stocks and closing prices into map" << std::endl;

				// Insert all stocks' previous day closing prices into the map
				for(uint64_t ticker_indx = 0, ticker_count = market_data_json["resultsCount"]; ticker_indx < ticker_count; ++ticker_indx) {
					

					std::string symb = market_data_json["results"][ticker_indx]["T"];

					float o = market_data_json["results"][ticker_indx]["o"];

					// If ticker has weird symbols, or if ticker has low volume, then don't insert
					if((symb.size() > 10) || (static_cast<uint64_t>(market_data_json["results"][ticker_indx]["v"]) < 400000) || /*(static_cast<uint64_t>(market_data_json["results"][ticker_indx]["v"]) > 1000000)  ||*/ (o<1) || (o>13)/*|| (o<1) || ( (o >3) && (o<4) ) || ( (o>6) && (o<7) ) || ( (o>10) && (o<12) ) || (o>14)*/) continue;

					prev_day_closing_price_map.emplace(symb, market_data_json["results"][ticker_indx]["c"]);
				}


				// Request the current day's opening price for all stocks
				request =	"https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/"
								
						+ 	 std::to_string(year) + "-" + ( ((month+1)<10) ? ("0"+std::to_string(month+1)) : std::to_string(month+1) ) + "-" 
							
						+ 	( (day<10) ? ("0"+std::to_string(day)) : std::to_string(day) )

						+ "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

// std::cout << "Requesting opening prices for all stocks on: " << month+1 << "-" << day << "-" << year << std::endl;

				fulfill_request(request, response);

				
				market_data_json = json::parse(response);

// std::cout << "Market was open? " << ((market_data_json["resultsCount"]==0) ? "False" : "True") << std::endl;


				// Market was closed on this day, so continue to next day.
				if(market_data_json["resultsCount"] == 0) continue;


				// The previous day closing price of the stock
				float previous_day_closing_price;

				// Check the percent change of all stocks from their previous closing price
				for(uint64_t ticker_indx = 0, ticker_count = market_data_json["resultsCount"]; ticker_indx < ticker_count; ++ticker_indx) {

					std::map<std::string, float>::iterator it = prev_day_closing_price_map.find(market_data_json["results"][ticker_indx]["T"]);

					// Ticker does not have a previous day closing price; It must be a new ticker.
					if(it == prev_day_closing_price_map.end()) continue;

					// The previous day closing price of the stock
					previous_day_closing_price = it->second;
// std::cout << market_data_json["results"][ticker_indx]["o"] << " closed at " << previous_day_closing_price << " the previous day." << std::endl;std::cout << market_data_json["results"][ticker_indx]["T"] << " closed at " << previous_day_closing_price << " the previous day." << std::endl;

					float percent_change = ((static_cast<float>(market_data_json["results"][ticker_indx]["o"]) - previous_day_closing_price)/previous_day_closing_price)*100;

					if(percent_change >= 5) {			

						request = 	"https://api.polygon.io/v2/aggs/ticker/" + it->first + "/range/1/minute/" 

								+  	std::to_string(get_time_in_ms(month, day, year, 8, 30, 0)) + "/" + std::to_string(get_time_in_ms(month, day, year, 12, 30, 0)) 
						
								+ 	"?unadjusted=false&sort=asc&limit=50000&apiKey=" STRINGIZE_VAL(APIKEYID); 

						fulfill_request(request, response);

						decltype(json::parse(response)) stock_spiked_json;

						try {

							stock_spiked_json = json::parse(response);
						}
						catch(nlohmann::detail::parse_error err) {

							std::cerr << "Part of response lost in transmission" << std::endl;

							++errors;

							continue;

						}

						if(stock_spiked_json["resultsCount"] < 3) continue;

						float open_spike_price = market_data_json["results"][ticker_indx]["o"];

						float peak_price = stock_spiked_json["results"][2]["c"];

						float low_of_peak = stock_spiked_json["results"][2]["l"];

						float dip_price = 0;

						float high_of_red = 0;

						float retract_price = 0;

						float low_of_green = 0;

						uint64_t minute_count = stock_spiked_json["resultsCount"];

						bool pass = true;

						uint64_t minute_indx1;


						if(stock_spiked_json["results"][0]["c"] <= stock_spiked_json["results"][0]["o"]) continue;

						if(stock_spiked_json["results"][1]["c"] <= stock_spiked_json["results"][1]["o"]) continue;

						if(stock_spiked_json["results"][2]["c"] <= stock_spiked_json["results"][2]["o"]) continue;

						for(minute_indx1 = 3; minute_indx1 < minute_count; ++minute_indx1) {
							
							float curr_minute_price = stock_spiked_json["results"][minute_indx1]["c"];


							// Price has fallen below today's open price: Does not qualify
							if((curr_minute_price < open_spike_price) || (curr_minute_price < low_of_peak) || ((minute_indx1 + 1) >= minute_count) ) {

								pass = false;
								break;
							}

							else if(curr_minute_price > open_spike_price) {

								if(curr_minute_price > peak_price) {
									
									peak_price = curr_minute_price;

									low_of_peak = stock_spiked_json["results"][minute_indx1]["l"];

								}

								else if(curr_minute_price < peak_price) {

									high_of_red = stock_spiked_json["results"][minute_indx1]["h"];
									
									dip_price = curr_minute_price;
									
									break;
								}
							}

							
							
						}

						if(!pass) continue;

						

						uint64_t minute_indx2;
						for(minute_indx2 = minute_indx1 + 1; minute_indx2 < minute_count; ++minute_indx2) {

							float curr_minute_price = stock_spiked_json["results"][minute_indx2]["c"];

							if((curr_minute_price < open_spike_price) || (curr_minute_price < low_of_peak) || ((minute_indx2 + 1) >= minute_count) ) {

								pass = false;
								break;

							}

							else if(curr_minute_price < dip_price) {
								
								dip_price = curr_minute_price;

								high_of_red = stock_spiked_json["results"][minute_indx2]["h"];

							}

							else if((curr_minute_price > dip_price) && (curr_minute_price > high_of_red)) {

std::cout << "D: " << stock_spiked_json["ticker"] << std::endl;

std::cout << "Bought " << stock_spiked_json["ticker"] << " at $" << high_of_red << std::endl;

								++denominator;
								
								retract_price = /*curr_minute_price;*/ high_of_red;

								low_of_green = stock_spiked_json["results"][minute_indx2]["l"];

								break;
								
							}

							else if((curr_minute_price > dip_price) && (curr_minute_price <= high_of_red)) continue;

						}

						if(!pass) continue;

						bool temp =false;
						uint64_t minute_indx3;
						for(minute_indx3 = minute_indx2 + 1; minute_indx3 < minute_count; ++minute_indx3) {

							float curr_minute_price = stock_spiked_json["results"][minute_indx3]["c"];

							

							if((curr_minute_price <= open_spike_price) || (curr_minute_price <= low_of_peak) || ((minute_indx3 + 1) >= minute_count)) {


std::cout << "Sold " << stock_spiked_json["ticker"] << " at $" << curr_minute_price << std::endl;

								uint64_t shares = (3000/retract_price);

								float pl = shares*(curr_minute_price - retract_price);

								gains += pl;
								pass = false;


								float percentage_gain = (curr_minute_price - retract_price)/retract_price;

								pl_percentages[pl_indx].emplace_back(percentage_gain);

								pl_percentages_y[pl_indx_y].emplace_back(percentage_gain);

								pl_percentages_m[pl_indx_m].emplace_back(percentage_gain);

								break;

							}

							else if(curr_minute_price >= retract_price) {
								
								low_of_green = stock_spiked_json["results"][minute_indx3]["l"];
								if(!temp) {++numerator;temp = true;}

								
							/*
								float further_percent_change = ((curr_minute_price - retract_price)/retract_price)*100;

								if(further_percent_change >= 10) {

std::cout << "N: " << stock_spiked_json["ticker"] << std::endl;


std::cout << "Sold " << stock_spiked_json["ticker"] << " at $" << curr_minute_price << std::endl;

									++numerator;


									uint64_t shares = (3000/retract_price);

									float pl = shares*(curr_minute_price - retract_price);

									gains += pl;



									break;
								}

								else {

									low_of_green = stock_spiked_json["results"][minute_indx3]["l"];
								}
								
								*/
							}

							else if(curr_minute_price <= low_of_green) {

std::cout << "Sold " << stock_spiked_json["ticker"] << " at $" << low_of_green << std::endl;

								uint64_t shares = (3000/retract_price);

								float pl = shares*(low_of_green - retract_price);

								gains += pl;

								float percentage_gain = (low_of_green - retract_price)/retract_price;

								pl_percentages[pl_indx].emplace_back(percentage_gain);

								pl_percentages_y[pl_indx_y].emplace_back(percentage_gain);

								pl_percentages_m[pl_indx_m].emplace_back(percentage_gain);
								
								break;

							}
							

						}


					}
					
				
				}

				prev_month = month;

				prev_day = day;

				prev_year = year;

			}


			

			for(uint64_t i = 0; i < 35; ++i) {

				
				uint64_t num_options = pl_percentages_m[i].size();

				if(num_options == 0) continue;

				float split_cap = real_gains_m/num_options;

				for(float p : pl_percentages_m[i]) {

					real_gains_m += (split_cap*p);
				}
				
			}

			std::cout << "Monthly GAINS/LOSSES: " << real_gains_m << std::endl;




		}

		float real_gains_y = 0;

		for(uint64_t i = 0; i < 365; ++i) {

			
			uint64_t num_options = pl_percentages_y[i].size();

			if(num_options == 0) continue;

			float split_cap = 50000/num_options;

			for(float p : pl_percentages_y[i]) {

				real_gains_y += (split_cap*p);
			}
			
		}

		std::cout << "YEARLY REAL GAINS: " << real_gains_y << std::endl;
	}

	uint64_t capital = 50000;

	float real_gains = 0;

	for(uint64_t i = 0; i < 2500; ++i) {

		
		uint64_t num_options = pl_percentages[i].size();

		if(num_options == 0) continue;

		float split_cap = capital/num_options;

		for(float p : pl_percentages[i]) {

			capital += (split_cap*p);
		}
		
	}

std::cout << "Number of responses lost in transmission: " <<  errors << std::endl;

std::cout << "Numerator (Number of times stock spiked 15% or more further after already spiking 10%-15%: " << numerator << std::endl; 
std::cout << "Denominator (Number of times stock spiked between 10%-15%): " << denominator << std::endl;
std::cout << "Computing probability: " << ((denominator!=0) ? std::to_string((static_cast<float>(numerator)/denominator)*100) : "No 10%-15% spike found.") << std::endl;
std::cout << "TOT LOSSES: " << losses << std::endl;
std::cout << "TOT WINS: " << gains << std::endl;
std::cout << "REAL GAINS: " << capital << std::endl;

// std::cout << "COUNT: " << count << std::endl;
// std::cout << "sum: " << sum << std::endl;
// std::cout << "Computing probability2: " << ((count!=0) ? std::to_string((static_cast<float>(sum)/count)) : "It's 0") << std::endl;
	// curl_easy_cleanup(curl);
	
    return 0;

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif
}
