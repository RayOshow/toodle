#pragma once

class token_control {

private:
	name self;
	

public:
	token_control(name _self) : self(_self) {
	}


	void send_token(name user1, name user2, asset quantity, string memo) {

		action(
			permission_level{self, "active"_n},
			TOKEN_CONTRACT, 
			"transfer"_n,
			std::make_tuple(
				user1, 
				user2, 
				quantity, 
				memo)
		).send();
	}	

	void stake(uint32_t user_seq, asset quantity) {

		require_auth(self);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );

		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ) {
			eosio_assert( row.unstaked_amount >= quantity.amount , ERROR_MSG_ID_NOT_ENOUGH_TOKEN);
	
			if(row.last_voting_charging_time == 0) {
				row.last_voting_charging_time = now();
			}				
					
			row.unstaked_amount -= quantity.amount;
			row.staked_amount += quantity.amount;
			row.voting_power += (quantity.amount / MAIN_TOKEN_SYMBOL_DECIMAL_N);
		});					
	}
	
	void begin_unstake(uint32_t unstake_seq, uint32_t user_seq, asset value) {
		
		require_auth(self);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ) {
			eosio_assert( row.staked_amount >= value.amount , ERROR_MSG_ID_NOT_ENOUGH_TOKEN);
			uint64_t unstake_voting_power = (value.amount / MAIN_TOKEN_SYMBOL_DECIMAL_N);
		
			row.staked_amount -= value.amount;

			if(row.voting_power <= unstake_voting_power ) {
				row.voting_power = 0;
			} 
			else {
				row.voting_power -= unstake_voting_power;
			}

			//////////////////////////////////////////////////////////////////////////////////
			// Add unstaking request.
			unstaking us;
			us.unstake_seq = unstake_seq;
			us.unstaking_amount = value.amount;
			us.start_time = now();
			us.end_flag = 0;
			row.unstaking_info.push_back(us);				
			//////////////////////////////////////////////////////////////////////////////////
		});	
	}
	
	void end_unstake(uint32_t unstake_seq, uint32_t user_seq) {
		require_auth(self);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto & row ) {
			int end_flag_cnt;
			int index;	
			uint8_t find_yn = 0;

			for ( index = 0; index < row.unstaking_info.size(); index++) {
				if(row.unstaking_info[index].unstake_seq == unstake_seq) {
					eosio_assert(row.unstaking_info[index].end_flag < 1, ERROR_MSG_UNSTAKE_ALREADY);

					// check unstaking time.
					uint32_t elapsed_time = now() - row.unstaking_info[index].start_time;
					eosio_assert(elapsed_time >= UNSTAKING_SEC, ERROR_MSG_STAKE_NOT_ENOUGH_TIME);

					// unstaking is done
					row.unstaked_amount += row.unstaking_info[index].unstaking_amount;

					// Set end flag.
					row.unstaking_info[index].end_flag = 1;
					
					find_yn = 1;	
				}

				if(row.unstaking_info[index].end_flag	> 0) {
					end_flag_cnt++;
				}			
			}

			if(end_flag_cnt >= index) {

				print("Delete all unstaking info");

				for (int j = 0; j < end_flag_cnt; j++) {						
					row.unstaking_info.pop_back();
				}
			}

			eosio_assert( find_yn , ERROR_MSG_STAKE_NO_INFO);	

		});		
	}
	
	void charge_voting_power(uint32_t user_seq) {
		
		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);
		
		acnts.modify(it, self, [&]( auto & row ) {		
			uint32_t time_gap = now() - row.last_voting_charging_time;
			
			eosio_assert(time_gap >= MIN_CHARGE_SEC, ERROR_MSG_VOTE_CHARGE_TIME_NOT_ENOUGH);
			
			// If time gap is over max time.
			if(time_gap >= MAX_CHARGE_SEC) {
				row.voting_power = row.staked_amount / MAIN_TOKEN_SYMBOL_DECIMAL_N;
				row.last_voting_charging_time = now();	
				return;
			}
			
			row.voting_power += ((row.staked_amount / MAIN_TOKEN_SYMBOL_DECIMAL_N) - row.voting_power) * time_gap / MAX_CHARGE_SEC;
			row.last_voting_charging_time = now();				
		});							
	}
	
	void vote(uint32_t content_seq, uint32_t voter_seq, int64_t voting_power, uint64_t spent_voting_power) {

		require_auth(self);

		///////////////////////////////////////////////////////////////////////
		// Udate Voting power
		///////////////////////////////////////////////////////////////////////
		accounts acnts(self, self.value);
		auto it = acnts.find( voter_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto & row ) {			
			eosio_assert(spent_voting_power <= row.voting_power, ERROR_MSG_CONTENTS_NOT_ENOUGH_VOTING_POWER);
			row.voting_power -= spent_voting_power;
			//row.last_voting_time = now();
			
		});	
		///////////////////////////////////////////////////////////////////////						

		///////////////////////////////////////////////////////////////////////
		// Udate Contents info
		///////////////////////////////////////////////////////////////////////
		contents cts(self, self.value);
		auto it2 = cts.find( content_seq );
		eosio_assert( it2 != cts.end(), ERROR_MSG_CONTENTS_NOT_EXIST);

		cts.modify(it2, self, [&]( auto& row ) {
			eosio_assert(row.writer_seq != voter_seq, ERROR_MSG_CONTENTS_VOTER_IS_WRITER);
			
			// Update content's voting info
			if(voting_power > 0 ) {
				row.up_voting_total += (uint64_t)spent_voting_power;	
				row.unproc_up_voting += (uint64_t)spent_voting_power;	
			}
			else {
				row.down_voting_total += (uint64_t)spent_voting_power;	
				row.unproc_down_voting += (uint64_t)spent_voting_power;	
			}

			int index;	
			
			for(index = 0; index < row.vote_info.size(); index++) {
				
				// Update user's voting power
				if(row.vote_info[index].user_seq == voter_seq) {
					row.vote_info[index].unproc_voting_total += voting_power;
					break;
				}
			}
			
			if(index == row.vote_info.size()) {
				//////////////////////////////////////////////////////////////////////////////
				// Add new voting request.
				ct_vote vi;
				vi.user_seq = voter_seq;
				vi.unproc_voting_total = voting_power;
				vi.reward_amount = 0;
				row.vote_info.push_back(vi);				
				//////////////////////////////////////////////////////////////////////////////
			}					
		});	
		
		///////////////////////////////////////////////////////////////////////				
	}
		
	void donate(uint32_t content_seq, uint32_t donater_seq, asset quantity) {
		
		require_auth(self);
		
		/////////////////////////////////////////////////////////////////////////////////////////
		// Sub balance from donator.
		/////////////////////////////////////////////////////////////////////////////////////////
		accounts acnts(self, self.value);
		auto it = acnts.find(donater_seq);
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto & row ) {		
			eosio_assert(row.unstaked_amount >= quantity.amount, ERROR_MSG_ID_NOT_ENOUGH_TOKEN);
			row.unstaked_amount -= quantity.amount;
		});	
		/////////////////////////////////////////////////////////////////////////////////////////
		
		/////////////////////////////////////////////////////////////////////////////////////////
		// Add reward 
		/////////////////////////////////////////////////////////////////////////////////////////
		contents cts(self, self.value);
		auto it2 = cts.find( content_seq );
		eosio_assert( it2 != cts.end(), ERROR_MSG_CONTENTS_NOT_EXIST);
		
		cts.modify(it2, self, [&]( auto & row ) {		
			row.reward_amount += quantity.amount;
			
			for( int index = 0; index < row.donate_info.size(); index++) {
	
				if(row.donate_info[index].user_seq == donater_seq) {
					row.donate_info[index].donate_total += quantity.amount;
					return;
				}
			}
			
			//////////////////////////////////////////////////////////////////////////////
			// Add new voting request.
			ct_donate dn;
			dn.user_seq = donater_seq;
			dn.donate_total = quantity.amount;
			row.donate_info.push_back(dn);				
			//////////////////////////////////////////////////////////////////////////////					
		});				
	}
};
