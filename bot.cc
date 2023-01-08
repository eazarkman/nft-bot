#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>

#include "include/json.hpp"
#include "include/HTTPRequest.hpp"

using namespace std;
using json = nlohmann::json;

template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string getPrice(string& str) {
	std::vector<std::string> result = split(str,' '); 
    if (result[1].compare("WAX") == 0){
		return result[0];
	}
	return "99999";
}

int main(int argc, char *argv[])
{
	double 	a = 0;
	int 	i = 0;
	double 	maxPrice = 2.02;
	 
	json 	data;
	json	asset_data;
	
	string	command;
	string  dprice;
	string 	template_id;
	string 	url;
	string  sale_id;
	string 	asset_id;
	string 	price;
	string  last_asset;
	ofstream myfile;
	
	
	
	try{
		while(true){
			
			http::Request request{"http://127.0.0.1/v1/chain/get_table_rows"};
			//http::Request request{"http://213.239.195.229/v1/chain/get_table_rows"};
			
			const string body = "{\"scope\":\"atomicmarket\",\"code\":\"atomicmarket\",\"table\":\"sales\",\"json\":true,\"limit\":1,\"reverse\":true}";
			const auto response = request.send("POST", body, {
				"Content-Type: application/json"		
			});
			
			a+=0.001;
			cout << a << '\n';
			
			data = json::parse(std::string{response.body.begin(), response.body.end()});
			
			for(i=0; i<data["rows"].size(); i++){
				if (data["rows"][i]["asset_ids"].size() == 1 && std::string{data["rows"][i]["collection_name"]}.compare("farmersworld") == 0) {
					
					sale_id  = to_string(data["rows"][i]["sale_id"]);
										
					asset_id = std::string{data["rows"][i]["asset_ids"][0]};
					
					if (asset_id.compare(last_asset) == 0) {
						continue;
					}
					
					price	 = std::string{data["rows"][i]["listing_price"]};
					dprice 	 = getPrice(price);
					if (stod(dprice) > maxPrice){
						continue;
					}
					
					url 	 = "http://priv-wax-us-proxy01.binfra.one:9000/atomicassets/v1/assets/"+asset_id;
					http::Request request{url};
					const auto response = request.send("GET");
					asset_data  = json::parse(std::string{response.body.begin(), response.body.end()});
					try{
						template_id = std::string{asset_data["data"]["template"]["template_id"]};
						if(template_id.compare("260676") == 0){
							//cout << asset_id + " is below maxPrice!\n";
							//cout   << dprice << endl;
							command = "node argv.js "+sale_id+" "+asset_id+" "+dprice+"WAX";
							system(command.c_str());
							//return 0;
						}
					}
					
					catch(const std::exception& e){
						cout << e.what() << '\n';
						continue;
					}
					last_asset = asset_id;
				}
			}
			
		}
	}
	catch (const std::exception& e)
	{
		cerr << "Request failed, error: " << e.what() << '\n';
		
	}
	
    return 0;
}
