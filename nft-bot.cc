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

void buy(string saleID,string assetID,string price) {
	/* unlock wallet */
	string cmd = "cleos wallet unlock --password PW5KNdQY6roCTrMKpuDSsKPUCWS1vDVS4wSfbBg6Apw1QJDQBHbMS";
	system(cmd.c_str());
	
	/* assertSale */
	cmd = "cleos push transaction \'{\"actions\": [{\"account\": \"atomicmarket\",\"name\": \"assertsale\",\"authorization\": [{\"actor\": \"anothertestt\",\"permission\": \"owner\"}],\"data\": {\"sale_id\":"+saleID+",\"asset_ids_to_assert\": [\""+assetID+"\"],\"listing_price_to_assert\": \""+price+"\",\"settlement_symbol_to_assert\": \"8,WAX\"}}]}\'";
	system(cmd.c_str());
		
	/* purchaseSale */ 
	cmd = "cleos push transaction \'{\"actions\": [{\"account\": \"atomicmarket\",\"name\": \"purchasesale\",\"authorization\": [{\"actor\": \"anothertestt\",\"permission\": \"owner\"}],\"data\": {\"buyer\":\"anothertestt\",\"sale_id\":"+saleID+",\"intended_delphi_median\":0,\"taker_marketplace\": \"\"}}]}\'";
	system(cmd.c_str());
	
	cout << "buying: "+saleID+", "+assetID+" @ "+price << endl;
}

int main(int argc, char *argv[])
{
	double 	maxPrice = 0.9;
	double 	a = 0;
	int 	i = 0;
	int 	template_id;
	json 	data;
	json	asset_data;
	string  seller;
	string	command;
	string  dprice;
	string  body;
	string 	url;
	string  sale_id;
	string 	asset_id;
	string 	price;
	string  last_asset;
	ofstream myfile;
		
	try{
		while(true){
			
			http::Request request{"http://127.0.0.1/v1/chain/get_table_rows"};
			body = "{\"scope\":\"atomicmarket\",\"code\":\"atomicmarket\",\"table\":\"sales\",\"json\":true,\"limit\":7,\"reverse\":true}";
			const auto response = request.send("POST", body, {
				"Content-Type: application/json"		
			});
			data = json::parse(std::string{response.body.begin(), response.body.end()});
			
			
			for(i=0; i<data["rows"].size(); i++){
				if (std::string{data["rows"][i]["collection_name"]}.compare("farmersworld") == 0) {
					
					price	 = std::string{data["rows"][i]["listing_price"]};
					dprice 	 = getPrice(price);

					if (stod(dprice) > maxPrice){
						continue;
					}

					sale_id  = to_string(data["rows"][i]["sale_id"]);				
					asset_id = std::string{data["rows"][i]["asset_ids"][0]};
					seller   = to_string(data["rows"][i]["seller"]);
					
					http::Request request{"http://127.0.0.1/v1/chain/get_table_rows"};
					body = "{\"scope\":"+seller+",\"code\":\"atomicassets\",\"table\":\"assets\",\"json\":true,\"lower_bound\":\""+asset_id+"\",\"upper_bound\":\""+asset_id+"\"}";
					const auto response = request.send("POST", body, {
						"Content-Type: application/json"		
					});

					asset_data  = json::parse(std::string{response.body.begin(), response.body.end()});
					
					try{
						template_id = asset_data["rows"][i]["template_id"];
						if(template_id == 260676) {
							buy(sale_id,asset_id,price);
						}
					}
					
					catch(const std::exception& e){
						//cout << e.what() << '\n';
						continue;
					}
					
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
