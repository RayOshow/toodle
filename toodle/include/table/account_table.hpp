#pragma once

/////////////////////////////////////////////////////////////////////////
// Account
struct unstaking {
	uint32_t unstake_seq;
	uint64_t unstaking_amount;
	uint32_t start_time;	
	uint8_t end_flag;
};

struct [[eosio::table , eosio::contract(CONTRACT_NAME)]] account {
	uint32_t user_seq; // user seq in web db
	uint64_t blockchain_id; // eos , fork or side chain account
	uint64_t staked_amount; 
	uint64_t unstaked_amount;	
	
	// unstake needs time.
	std::vector<unstaking> unstaking_info;

	uint64_t voting_power;
	//uint32_t last_voting_time;
	uint32_t last_voting_charging_time;

	uint32_t primary_key()const { return user_seq ;}

	EOSLIB_SERIALIZE(
			 account,
			 (user_seq)
			 (blockchain_id)
			 (staked_amount)
			 (unstaked_amount)
			 (unstaking_info)
			 (voting_power)
			 //(last_voting_time)
			 (last_voting_charging_time)
			 )
};

typedef eosio::multi_index< "account"_n, account > accounts;
