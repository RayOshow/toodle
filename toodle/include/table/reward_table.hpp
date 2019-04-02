#pragma once

struct rwpolicy {
	uint32_t reward_seq; 
	uint32_t limit_info; 
	uint8_t limit_type;			
	uint64_t total_amount;	
	uint64_t used_amount;	
	uint64_t reward_amount;	
	uint8_t status;
	uint32_t primary_key()const { return reward_seq;}	
};

// reward 
struct [[eosio::table , eosio::contract(CONTRACT_NAME)]] reward {
	uint32_t reward_type; 	// 리워드 종류		
	std::vector<rwpolicy> reward_policy; // 리워드 정책

	uint64_t total_amount; // 해당 리워드의 전체 보상액
	uint64_t total_used_amount; // 해당 리워드의 전체 지급액
	uint32_t primary_key()const { return reward_type;}	
};	

typedef eosio::multi_index< "reward"_n, reward > rewards;
