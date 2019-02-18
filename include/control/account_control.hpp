#pragma once

class account_control {

private:
	name self;

public:
	account_control(name self) : self(self) {
	}

	void signup(uint32_t user_seq, uint64_t reward_amount)
	{	 
		require_auth( self );

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it == acnts.end(), ERROR_MSG_ID_ALREADY_EXIST);

		acnts.emplace( self, [&]( auto& row ){
			// Get signup reward, if there exists.
			row.unstaked_amount = reward_amount;//get_rewards(REWARD_TYPE_SIGNUP);
			row.user_seq = user_seq;
			row.last_voting_charging_time = 0;
		});
	}
	
	void charge_token(uint32_t user_seq, asset quantity) 
	{		
		eosio_assert( quantity.symbol.code().to_string() == MAIN_TOKEN_SYMBOL,ERROR_MSG_SYMBOL_NOT_EXIST);
		
		accounts acnts(self, self.value);

		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ) {
			row.unstaked_amount += quantity.amount;	  					
		});			
	}	
	
	void set_blockchain_id(name blockchain_id, uint32_t user_seq) {
		
		require_auth(self);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ){ 				
			row.blockchain_id = blockchain_id.value;
		});		
	}
	
	void withdraw_token(name requestor, uint32_t user_seq, asset quantity) {
		
		require_auth(requestor);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ) {
			eosio_assert( row.blockchain_id != 0, ERROR_MSG_ID_NOT_EXIST);
			eosio_assert( requestor.value == row.blockchain_id, ERROR_MSG_ID_NOT_MATCH);
			eosio_assert( row.unstaked_amount >= quantity.amount, ERROR_MSG_ID_NOT_ENOUGH_TOKEN);
			
			row.unstaked_amount -= quantity.amount;
		});					
	}	
	
	void delete_id(uint32_t user_seq) {	 
	
		require_auth(self);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.erase(it);
	}
};
