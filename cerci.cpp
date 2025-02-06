#include <iostream>
#include <curl/curl.h>
#include <string>
#include <nlohmann/json.hpp> 
#include <cstdlib>  

using json = nlohmann::json; 
const std::string url = "************************";
const std::string api_key = "***************************";



size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}



int getResponse(std::string& response_data, const std::string& url, const std::string& api_key,std::string& user_demand) {
    CURL *curl;
    CURLcode result;

    
    curl = curl_easy_init();
    if (curl == NULL) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return -1;
    }

     
    const std::string json_data = R"({
        "messages": [{"role": "user", "content": "Please provide only the bash code needed to accomplish the following task. Do not include explanations, comments, or any information about the language used. Return only the command(s) which are not enclosed by anything. The task is to )"+user_demand+R"("}],
        "model": "gemma2-9b-it"
    })";

    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);

   
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

    

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        std::cerr << "Error: " << curl_easy_strerror(result) << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    std::cout << "Request performed successfully!" << std::endl;
    return 1;
}



void execute(std::string response_data){
    try {
        json json_response = json::parse(response_data);
        std::string message_content = json_response["choices"][0]["message"]["content"];

        if(message_content.substr(0,6)=="```bash") message_content = message_content.substr(7, message_content.size() - 9);
        std::cout << "Executing commands : \n" << "\033[32m" << message_content << "\033[0m" << std::endl;

        std::cout<< "Do you want to continue? [Y/n] ";
        char final;
        std::cin>>final;
        if(final=='y' || final=='Y') system(message_content.c_str());
        
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}




int main(int argc, char* argv[]) {

    std::string user_demand;

    if (argc < 2) {
        std::cerr << "Error: Please provide a string as an argument." << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        user_demand += argv[i];
        if (i < argc - 1) user_demand += " "; 
    }

    std::string response_data;
    if(!getResponse(response_data,url,api_key,user_demand)) return -1;
    
    execute(response_data);    

    return 0;
}
