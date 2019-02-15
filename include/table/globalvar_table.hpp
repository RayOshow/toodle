#pragma once

/////////////////////////////////////////////////////////////////////////
// Content
//contents _contents;
struct [[eosio::table , eosio::contract(CONTRACT_NAME)]] globalvar {
	uint32_t attr; 
	uint64_t value; 
	uint32_t primary_key()const { return attr ;}	
};		

typedef eosio::multi_index< "globalvar"_n, globalvar > globalvars;
