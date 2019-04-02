#pragma once

class content_control {

private:
	name self;

public:
	content_control(name _self) : self(_self) {
	}
	
	void register_content(uint32_t ct_seq, uint32_t writer_seq, uint64_t reward_amount) {

		require_auth(self);

		// Check if content's seq is new.
		contents cts(self, self.value);
		auto it = cts.find( ct_seq );
		eosio_assert( it == cts.end(), ERROR_MSG_CONTENTS_ALREADY_EXIST);

		// Check if account's seq exists.
		accounts acnts(self, self.value);
		auto it2 = acnts.find( writer_seq );
		eosio_assert( it2 != acnts.end(), ERROR_MSG_ID_NOT_EXIST);
		
		// Add new content info.
		cts.emplace( self, [&]( auto& row ){
			row.content_seq = ct_seq;	  
			row.writer_seq = writer_seq;
			row.write_time = now();
			// Get initial reward.
			row.reward_amount = reward_amount;
			
			// If there is reward, give it to writer.
			if(row.reward_amount > 0) {					
				acnts.modify(it2, self, [&]( auto& _row ) {														
					_row.unstaked_amount += row.reward_amount;
				});
			}					
		});
	}

	void delete_content(uint32_t content_seq) {

		require_auth(self);

		contents cts(self, self.value);
		auto it = cts.find( content_seq );
		eosio_assert( it != cts.end(), ERROR_MSG_CONTENTS_NOT_EXIST);

		cts.erase(it);
	}	
};
