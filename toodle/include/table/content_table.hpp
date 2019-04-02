#pragma once

		
// Donate info
struct ct_donate {
	uint32_t user_seq;
	uint64_t donate_total;
};

// Vote info
struct ct_vote {
	uint32_t user_seq;
	uint64_t proced_voting_total;
	uint64_t reward_amount;
	
	// voting total amount not to be processed.
	int64_t unproc_voting_total;
};

struct [[eosio::table , eosio::contract(CONTRACT_NAME)]] content {
	uint32_t content_seq; // content seq in web db.
	uint32_t writer_seq; // user seq who write contents down.
	uint32_t write_time; // write time

	std::vector<ct_donate> donate_info;
	std::vector<ct_vote> vote_info;

	// All voting amount.
	uint64_t up_voting_total;
	uint64_t down_voting_total;			
	
	// All voting amount unprocessed.
	uint64_t unproc_up_voting;
	uint64_t unproc_down_voting;		
					
	uint64_t reward_amount;

	uint32_t primary_key()const { return content_seq ;}				
};

typedef eosio::multi_index< "content"_n, content > contents;
