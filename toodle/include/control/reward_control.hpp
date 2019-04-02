#pragma once

class reward_control {

private:
	name self;

public:
	reward_control(name _self) : self(_self) {
	}
	
	void put_reward_pool(uint32_t reward_type,uint32_t reward_seq,asset quantity){
		
		eosio_assert( quantity.symbol.code().to_string() == MAIN_TOKEN_SYMBOL,ERROR_MSG_SYMBOL_NOT_EXIST);

		require_auth( REWARD_ACCOUNT );
		rewards rws(self, self.value);
		auto it = rws.find(reward_type);

		if(it == rws.end()) {

			rws.emplace( self, [&]( auto& row ){
				row.reward_type = reward_type;
				row.total_amount = quantity.amount;

				rwpolicy rwp;
				rwp.reward_seq = reward_seq;
				rwp.status = REWARD_STATUS_RESERVE;
				rwp.total_amount = quantity.amount;

				row.reward_policy.push_back(rwp);
			});
		}
		else {

			rws.modify(it, self, [&]( auto& row ) {
				
				row.total_amount += quantity.amount;

				for(int index = 0; index < row.reward_policy.size(); index++) {
					// If seq already exists,                            
					if(row.reward_policy[index].reward_seq == reward_seq) {
						row.reward_policy[index].total_amount += quantity.amount;
						return;
					}
				}

				rwpolicy rwp;
				rwp.reward_seq = reward_seq;
				rwp.status = REWARD_STATUS_RESERVE;
				rwp.total_amount = quantity.amount;
				row.reward_policy.push_back(rwp);
			});
		}
	}
	
	void set_reward_policy(uint32_t reward_type, uint32_t reward_seq, asset quantity, uint8_t limit_type, uint32_t limit_info) {
	
		require_auth( self );
		
		rewards rws(self, self.value);
		auto it = rws.find(reward_type);
		eosio_assert( it != rws.end(),ERROR_MSG_REWARD_NOT_EXIST);

		rws.modify(it, self, [&]( auto& row ) {	

			int index;
			
			for(index = 0; index < row.reward_policy.size(); index++) {

				if(row.reward_policy[index].reward_seq == reward_seq) {

					eosio_assert( row.reward_policy[index].total_amount > quantity.amount, ERROR_MSG_INVALID_REWARD_AMOUNT);
					
					row.reward_policy[index].status = REWARD_STATUS_USE;
					row.reward_policy[index].reward_amount = quantity.amount;	
					row.reward_policy[index].used_amount = 0;
					row.reward_policy[index].limit_type = limit_type;
					row.reward_policy[index].limit_info = limit_info;
					break;
				}
			}				

			eosio_assert( index != row.reward_policy.size(), ERROR_MSG_REWARD_NOT_EXIST);				
		});				
	}
	
	void do_vote_reward_process(uint64_t reward_amount,uint64_t unproc_up_vote_amount,uint64_t unproc_down_vote_amount){
	
		require_auth(self);
								
		eosio_assert( reward_amount > 0, ERROR_MSG_REWARD_NOT_ENOUGH);
		eosio_assert( unproc_up_vote_amount > unproc_down_vote_amount, ERROR_MSG_UNPROC_VOTE_NOT_ENOUGH);
		
		uint64_t unproc_vote_amount = unproc_up_vote_amount - unproc_down_vote_amount;
		uint64_t reward_ratio = reward_amount / unproc_vote_amount;
		
		contents cts(self, self.value);
					
		for (auto it = cts.cbegin(); it != cts.cend(); it++) {
			
			// When a content can get rewards.
			if((it->unproc_up_voting - it->unproc_down_voting) > 0) {
			
				cts.modify(it, self, [&]( auto& row ) {

					uint64_t unproc_voting_total = 0;
					
					// Caculate reward.
					uint64_t _reward_amount = reward_ratio * (row.unproc_up_voting - row.unproc_down_voting) ;
					uint64_t _reward_writer_amount = _reward_amount * REWARD_RATIO_WRITER;
					uint64_t _reward_voter_amount = _reward_amount - _reward_writer_amount;						
																
					// Caculate voting standard value
					for(int index = 0; index < row.vote_info.size(); index++) {
						if(row.vote_info[index].unproc_voting_total > 0 ) {
							unproc_voting_total += row.vote_info[index].unproc_voting_total;							
						}
					}
					
					//Give reward to writer
					////////////////////////////////////////////////////////////
					accounts acnts(self, self.value);
					auto it2 = acnts.find(row.writer_seq);
					eosio_assert( it2 != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

					acnts.modify(it2, self, [&]( auto & row2 ) {			
						row2.unstaked_amount += _reward_writer_amount;
					});	
					////////////////////////////////////////////////////////////
					
					// Give rewards to voters.
					for(int index = 0; index < row.vote_info.size(); index++) {
						if(row.vote_info[index].unproc_voting_total > 0 ) {
							uint64_t voter_reward = _reward_voter_amount * row.vote_info[index].unproc_voting_total / unproc_voting_total;
							
							/////////////////////////////////////////////////////////
							// Give reward to voter								
							it2 = acnts.find( row.vote_info[index].user_seq);
							eosio_assert( it2 != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

							acnts.modify(it2, self, [&]( auto & row2 ) {			
								row2.unstaked_amount += voter_reward;
							});	
							/////////////////////////////////////////////////////////
							
							// Init the value
							row.vote_info[index].reward_amount += voter_reward;
							row.vote_info[index].proced_voting_total += row.vote_info[index].unproc_voting_total;
							row.vote_info[index].unproc_voting_total = 0;								
						}	
						else {
							row.vote_info[index].unproc_voting_total = 0;
						}					
					}

					row.reward_amount += _reward_amount;
					row.unproc_up_voting =0;
					row.unproc_down_voting =0;
				});
			}
		}	
	}
	
	void delete_reward(uint32_t seq) {
		
		require_auth(self);

		rewards rwd(self, self.value);
		auto it = rwd.find( seq );
		eosio_assert( it != rwd.end(), ERROR_MSG_CONTENTS_NOT_EXIST);

		rwd.erase(it);
	}
	
	/**

			It's common func for getting reward.

	**/
	uint64_t get_rewards(uint32_t reward_type) {

		uint64_t reward_amount = 0;
		
		rewards rws(self, self.value);
		auto it = rws.find(reward_type);

		// if there is no reward type policy , reward will be zero.
		if(it == rws.end()) {
			return 0;
		}		

		rws.modify(it, self, [&]( auto& row ) {	
			for(int index = 0; index < row.reward_policy.size(); index++) {
				if(row.reward_policy[index].status == REWARD_STATUS_USE) {

					// If reward policy's time is out.
					if(row.reward_policy[index].limit_type == REWARD_LIMIT_TYPE1_TIME) {

						print(" limit type time: ");
						print(" now:", now());	
						print(" info:", row.reward_policy[index].limit_info);

						if(now() >= row.reward_policy[index].limit_info) {
							row.reward_policy[index].status = REWARD_STATUS_CLOSE;
							continue;
						}
					}

					// If reward policy's count is over.
					if(row.reward_policy[index].limit_type == REWARD_LIMIT_TYPE2_CNT) {

						print(" limit type cnt: ");
						print(" cnt:", row.reward_policy[index].used_amount/row.reward_policy[index].reward_amount);	
						print(" info:", row.reward_policy[index].limit_info);

						if((row.reward_policy[index].used_amount/row.reward_policy[index].reward_amount) >= row.reward_policy[index].limit_info) {
							row.reward_policy[index].status = REWARD_STATUS_CLOSE;
							continue;
						}
					}

					reward_amount = row.reward_policy[index].reward_amount;
					row.reward_policy[index].used_amount += reward_amount;

					if((row.reward_policy[index].total_amount <= row.reward_policy[index].used_amount) ||
						(row.reward_policy[index].total_amount - row.reward_policy[index].used_amount < row.reward_policy[index].reward_amount)) {
						row.reward_policy[index].status = REWARD_STATUS_CLOSE;
					}

					row.total_used_amount += reward_amount;

					print(" reward_amount: ", reward_amount);
					print(" used_amount: ", row.reward_policy[index].used_amount);
					print(" total_amount: ", row.reward_policy[index].total_amount);
					print(" status: ", row.reward_policy[index].status);

					break;
				}
			}	
		});

		print(" reward_amount: ", reward_amount);
		return reward_amount;
	}
};
