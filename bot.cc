/*
 * This code scans the WAX blockchain for NFT assets 
 * listed on AtomicHub.io for purchase given target
 * parameters coded below.
 *
 * Author: Erick Azarkman (erick@azarkman.com)
 *
 */

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
	double 	maxPrice = 16.00;
	int 	template_id;
	bool    first_time = true;
	json 	data;
	json	asset_data;
	string	command;
	string  dprice;
	string  body;

	string 	url;
	string  sale_id;
	string 	asset_id;
	string 	price;
	string  seller;
	string  last_offering;
	ofstream myfile;
	string endpoint = "http://wax.eosusa.io/v1/chain/"; //bot runs much faster running locally on a WAX node.
	
	
	try{
		while(true){
			
			http::Request request{endpoint + "get_table_rows"};
			
			if(first_time) {
				first_time = false;
				body = "{\"scope\":\"atomicmarket\",\"code\":\"atomicmarket\",\"table\":\"sales\",\"json\":true,\"limit\":1000,\"reverse\":true}";
			}else{
				body = "{\"scope\":\"atomicmarket\",\"code\":\"atomicmarket\",\"table\":\"sales\",\"json\":true,\"limit\":1,\"reverse\":true}";
			}
			const auto response = request.send("POST", body, {
				"Content-Type: application/json"		
			});
			
			data = json::parse(std::string{response.body.begin(), response.body.end()});
			sale_id = to_string(data["rows"][0]["sale_id"]);
			if(last_offering.compare(sale_id) == 0)
				continue;
			cout << "bot: scanning sale_id: "<< data["rows"][0]["sale_id"] << "... ";
			
			
			for(i=0; i<data["rows"].size(); i++){
				
				//target parameters defined here
				if (data["rows"][i]["asset_ids"].size() == 1){ //&& std::string{data["rows"][i]["collection_name"]}.compare("dinoworldwax") == 0) {
									
					sale_id  = to_string(data["rows"][i]["sale_id"]);				
					asset_id = std::string{data["rows"][i]["asset_ids"][0]};
					seller   = to_string(data["rows"][i]["seller"]);
					price	 = std::string{data["rows"][i]["listing_price"]};
					dprice 	 = getPrice(price);
					if (stod(dprice) > maxPrice){
						cout << "pass\n";
						continue;
					}
					cout << "within target parameters\n";
					http::Request request{"http://wax.eosusa.io/v1/chain/get_table_rows"};
					body = "{\"scope\":"+seller+",\"code\":\"atomicassets\",\"table\":\"assets\",\"json\":true,\"lower_bound\":\""+asset_id+"\",\"upper_bound\":\""+asset_id+"\"}";
					const auto response = request.send("POST", body, {
						"Content-Type: application/json"		
					});
					data  = json::parse(std::string{response.body.begin(), response.body.end()});
					try{
						cout << "seller: "+seller+"\ncollection name: " + std::string{data["rows"][0]["collection_name"]}+"\n";
						cout << "template_id: " << data["rows"][0]["template_id"] << "\n";
						cout << "asset id: " << data["rows"][0]["asset_id"] << "\n";
						cout << "asset price: " + price + "\n\n";
						
						template_id = data["rows"][0]["template_id"];
						if(template_id == 260676) {
							//execute purchase js script 
							command = "node purchase_assets.js "+sale_id+" "+asset_id+" "+dprice+"WAX";
							system(command.c_str());
						}
					}
					
					catch(const std::exception& e){
						cout << e.what() << '\n';
						continue;
					}
					
				}else{
					cout << "pass\n";
				}
			}
			last_offering = sale_id;
		}
	}
	catch (const std::exception& e)
	{
		cerr << "Request failed, error: " << e.what() << '\n';
		
	}
	
    return 0;
}
