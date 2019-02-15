#pragma once

class globalvar_control {

private:
	name self;

public:
	globalvar_control(name _self) : self(_self) {
	}
	
	void erase_gobal_var(uint32_t attr) {

		require_auth(self);

		globalvars gvars(self, self.value);
		auto it = gvars.find( attr );
		eosio_assert( it != gvars.end(), ERROR_MSG_CONTENTS_NOT_EXIST);

		gvars.erase(it);
	}
	
	
	void set_globalvars(uint32_t attr, uint64_t var, uint8_t update_add_flag) {

		globalvars gvars(self, self.value);

		auto it = gvars.find( attr );
		if(it == gvars.end()) {
			gvars.emplace( self, [&]( auto& row ){
				row.attr = attr;
				row.value = var;		  
			});
		}
		else {
			
			gvars.modify(it, self, [&]( auto& row ) {	
				if(update_add_flag > 0) {
					row.value += var;							
				}
				else {
					row.value = var;
				}
			});
		}
	}

	uint64_t get_globalvars(uint32_t attr, uint64_t default_val) {

		uint64_t rtn_value = 0;
		
		globalvars gvars(self, self.value);
		auto it = gvars.find( attr);

		if(it == gvars.end()) {
			gvars.emplace(self, [&]( auto& row ){
				row.attr = attr;
				row.value = default_val;	
			});

			rtn_value = default_val;
		}
		else {

			gvars.modify(it, self, [&]( auto& row ) {	
				rtn_value = row.value;					
			});
		}
		return rtn_value;
	}
};
