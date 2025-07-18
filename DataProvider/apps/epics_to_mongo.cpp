#include <iostream>
#include <stdio.h>
#include <curl/curl.h>

size_t write_callback(void *contents, size_t size, size_t nmemb, FILE *file){
    return fwrite(contents, size, nmemb, file);
}

int main(void){
    CURL *curl;
    CURLcode result;
    FILE *file;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl){
        // First request
        file = fopen("response1.json", "wb");
        if(file) {
            curl_easy_setopt(curl, CURLOPT_URL, "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json?pv=IOC%3ABSY0%3AMP01%3APC_RATE&from=2025-06-23T00%3A00%3A00.000Z&to=2025-06-23T23%3A59%3A59.000Z&fetchLatestMetadata=true");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            result = curl_easy_perform(curl);
            fclose(file);
            
            if(result != CURLE_OK) {
                std::cerr << "Error during first curl request: " << curl_easy_strerror(result) << std::endl;
            }
        }
        
        // Second request
        file = fopen("response2.json", "wb");
        if(file) {
            curl_easy_setopt(curl, CURLOPT_URL, "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json?pv=IOC%3AIN20%3AEV01%3ARG02_ACTRATE&from=2025-07-14T00%3A00%3A00.000Z&to=2025-07-14T23%3A59%3A59.000Z&fetchLatestMetadata=true");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            result = curl_easy_perform(curl);
            fclose(file);
            
            if(result != CURLE_OK) {
                std::cerr << "Error during second curl request: " << curl_easy_strerror(result) << std::endl;
            }
        }
        
        // Third request
        file = fopen("response3.json", "wb");
        if(file) {
            curl_easy_setopt(curl, CURLOPT_URL, "http://lcls-archapp.slac.stanford.edu/retrieval/data/getData.json?pv=STPR%3ABSYH%3A2%3ASTD2_IN_A&from=2025-07-14T00%3A00%3A00.000Z&to=2025-07-14T23%3A59%3A59.000Z&fetchLatestMetadata=true");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            result = curl_easy_perform(curl);
            fclose(file);
            
            if(result != CURLE_OK) {
                std::cerr << "Error during third curl request: " << curl_easy_strerror(result) << std::endl;
            }
        }
        
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Error initializing curl." << std::endl;
    }
    
    curl_global_cleanup();
    return 0;
}
