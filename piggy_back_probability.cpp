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
#define GL_VEC_SZ 366
#define GL_VEC_SZ_MO 35


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
struct Data {

    float price;

    float volume; 
};

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

int main(void) {


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

    uint64_t worst_numerator = 0;

    uint64_t best_numerator = 0;

    uint64_t mid_numerator = 0;

	// For all stocks, number of 10%-15% spikes that occurred.
	uint64_t denominator = 0;

    std::map<std::string, std::vector<float>> percent_GL_vecs[GL_VEC_SZ];


	float gains = 0;

	float losses = 0;

	uint64_t errors = 0;

    uint64_t percent_GL_indx = 0;

    float worst_cap_mo = 5000;
    float best_cap_mo  = 5000;
    float mid_cap_mo   = 5000;

    float tot = 0;

	// Years range from [2018, 2021)
	for(uint64_t year = 2020; year < 2021; ++year) {

		// If leap-year, adjust February accordingly.
		if(year%4 == 0) months[1] = 29;

		else months[1] = 28;

		// Months range from [0,11]
		for(uint64_t  month = 0; month < 12; ++month) {

            std::map<std::string, std::vector<float>> percent_GL_vecs_mo[GL_VEC_SZ_MO];

            uint64_t percent_GL_indx_mo = 0;


			// Days range from [1,30], [1,31], [1,28], or [1,29] depending on month and if leap-year or not.
			for(uint64_t day = 1, days_in_month = months[month]; day <= days_in_month; ++day, ++percent_GL_indx, ++percent_GL_indx_mo) {


std::cout << month+1 << "/" << day << "/" << year << std::endl;				

				std::string response;

				std::map<std::string, Data> prev_day_closing_price_map;

				// Request the Previous day's closing price for all stocks
				std::string request =	 "https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/"
								
									+ 	 std::to_string(prev_year) + "-" + ( ((prev_month+1)<10) ? ("0"+std::to_string(prev_month+1)) : std::to_string(prev_month+1) ) + "-" 
							
									+ 	( (prev_day<10) ? ("0"+std::to_string(prev_day)) : std::to_string(prev_day) )

									+ "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

// std::cout << "Making request for previous day closing price for all stocks: \n" << request << std::endl;

				fulfill_request(request, response);

                decltype(json::parse(response)) market_data_json;

                while(true) {

                    try {

                        market_data_json = json::parse(response);

                        break;
                    }
                    catch(nlohmann::detail::parse_error err) {

                        std::cerr << "Part of response lost in transmission" << std::endl;

                        ++errors;

                    }
                }




				// Either today's date, future date, or market was closed: Cannot get day's closing price.
				if(market_data_json["resultsCount"] == 0) continue;

// std::cout << "Inserting stocks and closing prices into map" << std::endl;

				// Insert all stocks' previous day closing prices into the map
				for(uint64_t ticker_indx = 0, ticker_count = market_data_json["resultsCount"]; ticker_indx < ticker_count; ++ticker_indx) {
					
					std::string symb = market_data_json["results"][ticker_indx]["T"];

					// If ticker has weird symbols, or if ticker has low volume, then don't insert
					if(symb.size() > 10) continue;

                    Data data;

                    data.price = market_data_json["results"][ticker_indx]["c"];

                    data.volume = market_data_json["results"][ticker_indx]["v"];

					prev_day_closing_price_map.emplace(symb,data);
				}


				// Request the current day's opening price for all stocks
				request =	"https://api.polygon.io/v2/aggs/grouped/locale/us/market/stocks/"
								
						+ 	 std::to_string(year) + "-" + ( ((month+1)<10) ? ("0"+std::to_string(month+1)) : std::to_string(month+1) ) + "-" 
							
						+ 	( (day<10) ? ("0"+std::to_string(day)) : std::to_string(day) )

						+ "?unadjusted=false&apiKey=" STRINGIZE_VAL(APIKEYID);

// std::cout << "Requesting opening prices for all stocks on: " << month+1 << "-" << day << "-" << year << std::endl;

				fulfill_request(request, response);

				
                while(true) {

                    try {

                        market_data_json = json::parse(response);
                        break;
                    }
                    catch(nlohmann::detail::parse_error err) {

                        std::cerr << "Part of response lost in transmission" << std::endl;

                        ++errors;
                    }

                }
// std::cout << "Market was open? " << ((market_data_json["resultsCount"]==0) ? "False" : "True") << std::endl;


				// Market was closed on this day, so continue to next day.
				if(market_data_json["resultsCount"] == 0) continue;

                
				// The previous day closing price of the stock
				float previous_day_closing_price;

                float previous_day_volume;

				// Check the percent change of all stocks from their previous closing price
				for(uint64_t ticker_indx = 0, ticker_count = market_data_json["resultsCount"]; ticker_indx < ticker_count; ++ticker_indx) {

					std::map<std::string, Data>::iterator it = prev_day_closing_price_map.find(market_data_json["results"][ticker_indx]["T"]);

					// Ticker does not have a previous day closing price; It must be a new ticker.
					if(it == prev_day_closing_price_map.end()) continue;

					// The previous day closing price of the stock
					previous_day_closing_price = (it->second).price;

                    previous_day_volume = (it->second).volume;

// std::cout << market_data_json["results"][ticker_indx]["o"] << " closed at " << previous_day_closing_price << " the previous day." << std::endl;std::cout << market_data_json["results"][ticker_indx]["T"] << " closed at " << previous_day_closing_price << " the previous day." << std::endl;

					float price_percent_change = ((static_cast<float>(market_data_json["results"][ticker_indx]["o"]) - previous_day_closing_price)/previous_day_closing_price)*100;

                    float volume_percent_change = ((static_cast<float>(market_data_json["results"][ticker_indx]["v"]) - previous_day_volume)/previous_day_volume)*100;

					if((price_percent_change >= 5) && (volume_percent_change >= 500) && (previous_day_volume > 150000)) {			

						request = 	"https://api.polygon.io/v2/aggs/ticker/" + it->first + "/range/1/minute/" 

								+  	std::to_string(get_time_in_ms(month, day, year, 8, 30, 0)) + "/" + std::to_string(get_time_in_ms(month, day, year, 9, 0, 0)) 
						
								+ 	"?unadjusted=false&sort=asc&limit=50000&apiKey=" STRINGIZE_VAL(APIKEYID); 


						fulfill_request(request, response);

						decltype(json::parse(response)) stock_spiked_json;

                        while(true) {

                            try {

                                stock_spiked_json = json::parse(response);
                                break;
                            }
                            catch(nlohmann::detail::parse_error err) {

                                std::cerr << "Part of response lost in transmission" << std::endl;

                                ++errors;

                            }
                        }

                        if(stock_spiked_json["resultsCount"] <= 0) continue;
                        // if(stock_spiked_json["results"][0]["c"] <= stock_spiked_json["results"][0]["o"]) continue;

                        float open_minute_close_price = stock_spiked_json["results"][0]["c"], 
                              open_minute_open_price  = stock_spiked_json["results"][0]["o"], 
                              open_minute_high_price  = stock_spiked_json["results"][0]["h"], 
                              open_minute_low_price   = stock_spiked_json["results"][0]["l"], 
                              open_current_vwap       = stock_spiked_json["results"][0]["vw"];    

                        std::string ticker = stock_spiked_json["ticker"]; 

                        for(uint64_t minute_indx = 0, minute_count = stock_spiked_json["resultsCount"]; minute_indx < minute_count; /*++minute_indx*/) {

                            
                        

                            float minute_close_price    = stock_spiked_json["results"][minute_indx]["c"],
                                  minute_open_price     = stock_spiked_json["results"][minute_indx]["o"],
                                  minute_high_price     = stock_spiked_json["results"][minute_indx]["h"],
                                  minute_low_price      = stock_spiked_json["results"][minute_indx]["l"],
                                  current_vwap          = stock_spiked_json["results"][minute_indx]["vw"],
                                  best_case_buy_price   = -1,
                                  worst_case_buy_price  = -1,
                                  mid_case_buy_price    = -1,
                                  best_case_sell_price  = -1,
                                  worst_case_sell_price = -1,
                                  mid_case_sell_price   = -1;
                                  

                            /*if(minute_low_price < open_minute_open_price) break;*/

                            current_vwap -= (current_vwap*0.02);

                            if((minute_low_price <= current_vwap) /*&& (minute_low_price >= open_minute_open_price)*/) {

                                best_case_buy_price  = minute_low_price;

                                mid_case_buy_price   = /*current_vwap*/ minute_low_price + ((minute_high_price-minute_low_price)/2);

                                worst_case_buy_price = minute_high_price;

                                std::cout << month+1 << "-" << day << "-" << year << " Bought " << ticker << " @ " << mid_case_buy_price << std::endl << std::endl;
 
                                ++denominator;

                                float new_minute_indx = minute_indx+1;

                                float breakpoint_low_price = minute_low_price;
                                // breakpoint_low_price -= (breakpoint_low_price*0.01);
                                while(new_minute_indx  < minute_count) {

                                    float new_minute_high_price = stock_spiked_json["results"][new_minute_indx]["h"];
                                    float new_minute_low_price = stock_spiked_json["results"][new_minute_indx]["l"];
                                    
                                    

                                    if(new_minute_low_price < breakpoint_low_price) {

                                        worst_case_sell_price = new_minute_low_price;
                                        best_case_sell_price  = new_minute_high_price;
                                        mid_case_sell_price   = breakpoint_low_price;

                                        tot += (mid_case_sell_price - mid_case_buy_price);

                                        std::cout << month+1 << "-" << day << "-" << year << " Sold " << ticker << " @ " << mid_case_sell_price << std::endl << std::endl;


                                        float worst_case_percent_GL = ((worst_case_sell_price - worst_case_buy_price)/worst_case_buy_price);
                                        float best_case_percent_GL = ((best_case_sell_price - best_case_buy_price)/best_case_buy_price);
                                        float mid_case_percent_GL = ((mid_case_sell_price - mid_case_buy_price)/mid_case_buy_price);
                                        
                                        

                                        percent_GL_vecs[percent_GL_indx][ticker].emplace_back(worst_case_percent_GL);
                                        percent_GL_vecs[percent_GL_indx][ticker].emplace_back(best_case_percent_GL);
                                        percent_GL_vecs[percent_GL_indx][ticker].emplace_back(mid_case_percent_GL);

                                        percent_GL_vecs_mo[percent_GL_indx_mo][ticker].emplace_back(worst_case_percent_GL);
                                        percent_GL_vecs_mo[percent_GL_indx_mo][ticker].emplace_back(best_case_percent_GL);
                                        percent_GL_vecs_mo[percent_GL_indx_mo][ticker].emplace_back(mid_case_percent_GL);

                                        if(worst_case_percent_GL > 0.01) {

                                            ++worst_numerator;
                                        }
                                        if(best_case_percent_GL > 0.01) {

                                            ++best_numerator;
                                        }
                                        if(mid_case_percent_GL > 0.01) {

                                            ++mid_numerator;
                                        }

                                        minute_indx = new_minute_indx;

                                        break;
                                    }

                                    else if(new_minute_low_price > breakpoint_low_price) {

                                        breakpoint_low_price = new_minute_low_price;
                                        // breakpoint_low_price -= (breakpoint_low_price*0.01);
                                    }

           

                                    ++new_minute_indx;
                                }
                                
                                // Still holding positions and at end of day, so sell all positions.
                                if(minute_indx != new_minute_indx) {

                                    float new_minute_high_price = stock_spiked_json["results"][new_minute_indx-1]["h"];
                                    float new_minute_low_price = stock_spiked_json["results"][new_minute_indx-1]["l"];

                                    worst_case_sell_price = new_minute_low_price;
                                    best_case_sell_price  = new_minute_high_price;
                                    mid_case_sell_price   = breakpoint_low_price;

                                    tot += (mid_case_sell_price - mid_case_buy_price);

                                    std::cout << month+1 << "-" << day << "-" << year << " Sold " << ticker << " @ " << mid_case_sell_price << std::endl << std::endl;

                                    float worst_case_percent_GL = ((worst_case_sell_price - worst_case_buy_price)/worst_case_buy_price);
                                    float best_case_percent_GL = ((best_case_sell_price - best_case_buy_price)/best_case_buy_price);
                                    float mid_case_percent_GL = ((mid_case_sell_price - mid_case_buy_price)/mid_case_buy_price);

                                    percent_GL_vecs[percent_GL_indx][ticker].emplace_back(worst_case_percent_GL);
                                    percent_GL_vecs[percent_GL_indx][ticker].emplace_back(best_case_percent_GL);
                                    percent_GL_vecs[percent_GL_indx][ticker].emplace_back(mid_case_percent_GL);

                                    percent_GL_vecs_mo[percent_GL_indx_mo][ticker].emplace_back(worst_case_percent_GL);
                                    percent_GL_vecs_mo[percent_GL_indx_mo][ticker].emplace_back(best_case_percent_GL);
                                    percent_GL_vecs_mo[percent_GL_indx_mo][ticker].emplace_back(mid_case_percent_GL);

                                    if(worst_case_percent_GL > 0.01) {

                                        ++worst_numerator;
                                    }
                                    if(best_case_percent_GL > 0.01) {

                                        ++best_numerator;
                                    }
                                    if(mid_case_percent_GL > 0.01) {

                                        ++mid_numerator;
                                    }

                                    break; // Break out of for-loop


                                } 

                                else continue; // Do not increment minute_indx

                                // break


                            } // bought

                            ++minute_indx;
                        }

					}
					
				
				}

				prev_month = month;

				prev_day = day;

				prev_year = year;

			}

            std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

            std::string MO[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

            std::cout << "====================" << MO[month] << " Results" << "====================" << std::endl;

            float old_worst_cap_mo = worst_cap_mo;
            float old_best_cap_mo = best_cap_mo;
            float old_mid_cap_mo = mid_cap_mo;

            uint64_t dy = 1;

            for(uint64_t i = 0; i < GL_VEC_SZ_MO; ++i) {

                uint64_t opportunities = percent_GL_vecs_mo[i].size();

                std::cout << "DAY: " << dy++ << std::endl;

                if(dy >= months[month]) dy = 1;

                std::cout << "Opportunities: " << opportunities << std::endl;

                float worst_cap_distrib = worst_cap_mo/opportunities;
                float best_cap_distrib  = best_cap_mo/opportunities;
                float mid_cap_distrib   = mid_cap_mo/opportunities;

                for(auto& vec_pair : percent_GL_vecs_mo[i]) {

                    std::string ticker = vec_pair.first;

                    auto& vec = vec_pair.second;

                    std::cout << "Trading " << ticker << " " << vec.size()/3 << " times" << std::endl;

                    for(uint64_t j = 0, len = vec.size(); j+2 < len; j+=3) {

                        float worst_percent = vec[j];
                        float best_percent  = vec[j+1];
                        float mid_percent   = vec[j+2];

                        /*

                        std::cout << "Worst % " << worst_percent << std::endl;

                        std::cout << "Best % " << best_percent << std::endl;

                        std::cout << "Mid % " << mid_percent << std::endl;

                        */

                        worst_cap_mo += (worst_cap_distrib*worst_percent);
                        best_cap_mo  += (best_cap_distrib*best_percent);
                        mid_cap_mo   += (mid_cap_distrib*mid_percent);


                    }
                }

                std::cout << "Day's End Worst Cap: " << worst_cap_mo << std::endl;
                std::cout << "Day's End Best Cap: " << best_cap_mo << std::endl;
                std::cout << "Day's End Mid Cap: " << mid_cap_mo << std::endl;
            }

            
            std::cout << "******************** " << "Month's Worst GL: " << (worst_cap_mo - old_worst_cap_mo) << " ********************" << std::endl;

            std::cout << "******************** " << "Month's Best GL: " << (best_cap_mo - old_best_cap_mo) << " ********************" << std::endl;

            std::cout << "******************** " << "Month's Mid GL: " << (mid_cap_mo - old_mid_cap_mo) << " ********************" << std::endl;

            std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


		}



	}

    std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "Numerator: " << numerator << std::endl;
    std::cout << "Denominator: " << denominator << std::endl;
    std::cout << "Probability: " << ((denominator != 0) ? (std::to_string((static_cast<float>(numerator)/denominator)*100) + "%") : "0%") << std::endl;

    std::cout << "Best Numerator: " << best_numerator << std::endl;
    std::cout << "Best Case Probability: " << ((denominator != 0) ? (std::to_string((static_cast<float>(best_numerator)/denominator)*100) + "%") : "0%") << std::endl;

    std::cout << "Worst Numerator: " << worst_numerator << std::endl;
    std::cout << "Worst Case Probability: " << ((denominator != 0) ? (std::to_string((static_cast<float>(worst_numerator)/denominator)*100) + "%") : "0%") << std::endl;
	

    std::cout << "Mid Numerator: " << mid_numerator << std::endl;
    std::cout << "Mid Case Probability: " << ((denominator != 0) ? (std::to_string((static_cast<float>(mid_numerator)/denominator)*100) + "%") : "0%") << std::endl;
    std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


    float worst_cap = 5000;
    float best_cap  = 5000;
    float mid_cap   = 5000;

    for(uint64_t i = 0; i < GL_VEC_SZ; ++i) {

        uint64_t opportunities = percent_GL_vecs[i].size();

        // std::cout << "Opportunities: " << opportunities << std::endl;

        float worst_cap_distrib = worst_cap/opportunities;
        float best_cap_distrib  = best_cap/opportunities;
        float mid_cap_distrib   = mid_cap/opportunities;

        for(auto& vec_pair : percent_GL_vecs[i]) {

            std::string ticker = vec_pair.first;


            auto& vec = vec_pair.second;

            // std::cout << "Trading " << ticker << " " << vec.size()/3 << " times" << std::endl;

            for(uint64_t j = 0, len = vec.size(); j+2 < len; j+=3) {

                float worst_percent = vec[j];
                float best_percent  = vec[j+1];
                float mid_percent   = vec[j+2];

                /*
                std::cout << "Worst % " << worst_percent << std::endl;

                std::cout << "Best % " << best_percent << std::endl;

                std::cout << "Mid % " << mid_percent << std::endl;
                */

                worst_cap += (worst_cap_distrib*worst_percent);
                best_cap  += (best_cap_distrib*best_percent);
                mid_cap   += (mid_cap_distrib*mid_percent);


            }
        }
    }

    std::cout << "Worst Cap: " << worst_cap << std::endl;
    std::cout << "Best Cap: " << best_cap << std::endl;
    std::cout << "Mid Cap: " << mid_cap << std::endl;
    
    std::cout << "Worst GL: " << (worst_cap - 5000) << std::endl;

    std::cout << "Best GL: " << (best_cap - 5000) << std::endl;

    std::cout << "Mid GL: " << (mid_cap - 5000) << std::endl;

    std::cout << "TOT: " << tot << std::endl;

    std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


    return 0;

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make API-KEY-ID={YOUR_API_KEY_ID} API-SECRET-KEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif
}
