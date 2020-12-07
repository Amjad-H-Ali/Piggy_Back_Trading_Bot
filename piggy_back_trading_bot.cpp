#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <json.hpp>

// For convenience
using json = nlohmann::json;

#define STR(X) #X
#define STRINGIZE_VAL(X) STR(X)



// Writes JSON response object to stdout
static size_t write_data(const char* in, std::size_t size, std::size_t num, char* out) {

	std::string data(in, size*num);

    auto market_data_json = json::parse(data);

    std::cout << "JSON DUMP: " << std::endl;
    std::cout << market_data_json.dump(4) << std::endl;

    std::cout << market_data_json["ticker"] << std::endl;

	return size * num;
}

int main() {

#if defined (APIKEYID) &&  defined (APISECRETKEY)
    std::cout << "API-KEY-ID: " << STRINGIZE_VAL(APIKEYID) << std::endl;
    std::cout << "API-SECRET-KEY: " << STRINGIZE_VAL(APISECRETKEY) << std::endl;

	std::string request = "https://api.polygon.io/v2/aggs/ticker/AAPL/range/1/day/2019-01-01/2019-02-01?apiKey="  STRINGIZE_VAL(APIKEYID);


	std::cout << "request: " + request << std::endl;

    // Set HTTP header
	CURL *curl = curl_easy_init();
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
	unsigned http_code = 0;
	std::string http_data;
	
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_data);

	curl_easy_perform(curl);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

	curl_easy_cleanup(curl);


    return 0;

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make -DAPIKEYID={YOUR_API_KEY_ID} -DAPISECRETKEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 1;
#endif
}