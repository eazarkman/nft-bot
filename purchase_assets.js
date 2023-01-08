const myArgs = process.argv.slice(2);

const config    = require('config');
const fetch     = require('node-fetch');

const { Api, JsonRpc, RpcError } = require('eosjs');
const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig'); // development only
const { TextDecoder, TextEncoder } = require('util'); //node only

//const ecc = require('eosjs-ecc');

const privateKey1 = config.get('wallet.privateKey');
const privateKeys = [privateKey1];
const signatureProvider = new JsSignatureProvider(privateKeys);
const rpc = new JsonRpc('http://wax.eosusa.io/', { fetch }); //required to read blockchain state
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });

console.log('myArgs: ', myArgs);


sale_id  = myArgs[0];
asset_id = myArgs[1];
price    = myArgs[2];

async function assertSale(sale_id, asset_id, price) {
   return (async () => {
      await api.transact({
         actions: [{
            account: 'atomicmarket',
            name: 'assertsale',
            authorization: [{
               actor: 'thisisatestt',
               permission: 'owner',
            }],
            data: {
               "sale_id": sale_id,
               "asset_ids_to_assert": [
                  asset_id
               ],
               "listing_price_to_assert": price,
               "settlement_symbol_to_assert": "8,WAX"
            }
         }]
      },{
         blocksBehind: 3,
         expireSeconds: 30,
      })
   })();
}
async function purchaseSale(sale_id){
   return (async () => {
      await api.transact({
         actions: [{
            account: 'atomicmarket',
            name: 'purchasesale',
            authorization: [{
               actor: 'thisisatestt',
               permission: 'owner',
            }],
            data: {
               "buyer": "thisisatestt",
               "sale_id": sale_id,
               "intended_delphi_median": 0,
               "taker_marketplace": ""
            }
         }]
      },{
         blocksBehind: 3,
         expireSeconds: 30,
      })
   })();
}

assertSale(sale_id,asset_id,price);
purchaseSale(sale_id);

