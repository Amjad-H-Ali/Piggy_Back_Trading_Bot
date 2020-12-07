#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <json.hpp>

// For convenience
using json = nlohmann::json;

#define STR(X) #X
#define STRINGIZE_VAL(X) STR(X)

int main() {

#if defined (APIKEYID) &&  defined (APISECRETKEY)
    std::cout << "API-KEY-ID: " << STRINGIZE_VAL(APIKEYID) << std::endl;
    std::cout << "API-SECRET-KEY: " << STRINGIZE_VAL(APISECRETKEY) << std::endl;

	std::string request = "https://api.polygon.io/v2/aggs/ticker/AAPL/range/1/day/2019-01-01/2019-02-01?apiKey="  STRINGIZE_VAL(APIKEYID);


	std::cout << "request: " + request << std::endl;
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, request.c_str());
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,20);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	struct curl_slist *chunk = NULL;
	std::string keyid = "APCA-API-KEY-ID: " STRINGIZE_VAL(APIKEYID);
	std::string secret = "APCA-API-SECRET-KEY: " STRINGIZE_VAL(APISECRETKEY);

    std::cout << secret << std::endl;
	chunk = curl_slist_append(chunk, keyid.c_str());
	chunk = curl_slist_append(chunk, secret.c_str());

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

	//Response information
	long httpCode(0);
	std::string httpData;
	
	

    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

    
	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpData);


	curl_easy_perform(curl);

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

	curl_easy_cleanup(curl);

	std::cout << "httpCode: " << httpCode << std::endl;
    std::cout << "httpdata: " << httpData << std::endl;

#else   
    std::cerr << "API-KEY-ID and/or API-SECRET-KEY are/is undefined."  << std::endl;
    std::cerr << "Usage: >> $ make -DAPIKEYID={YOUR_API_KEY_ID} -DAPISECRETKEY={YOUR_API_SECRET_KEY}"  << std::endl;
    return 0;
#endif
}