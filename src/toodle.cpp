#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/dispatcher.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.hpp>
#include <vector>

using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::print;
using eosio::name;
using eosio::datastream;
using eosio::unpack_action_data;
using eosio::transaction;
using eosio::checksum256;

using std::string;
using std::vector;

#include "toodle.hpp"

// Table
#include "table/account_table.hpp"
#include "table/content_table.hpp"
#include "table/globalvar_table.hpp"
#include "table/reward_table.hpp"

// Contorller
#include "control/account_control.hpp"
#include "control/reward_control.hpp"
#include "control/token_control.hpp"
#include "control/globalvar_control.hpp"
#include "control/content_control.hpp"

/**
 @class 
 @date 2019/02/15
 @Author Ray-OShow(raymond@todos.co.kr)
 @brief  
**/

class [[eosio::contract]] toodle : public eosio::contract {

	private:

		account_control account_controller;
		reward_control reward_controller;
		token_control token_controller;		
		globalvar_control globalvar_controller;
		content_control content_controller;

	public:

		using contract::contract;

		toodle(name receiver, name code,  datastream<const char*> ds)
			: contract(receiver, code, ds ) 
			,account_controller(_self)
			,reward_controller(_self)
			,token_controller(_self)
			,globalvar_controller(_self)
			,content_controller(_self){
		}

		/** 
			Sign up
		**/
		[[eosio::action]]
		void signup(uint32_t user_seq){	 
			account_controller.signup(user_seq ,reward_controller.get_rewards(REWARD_TYPE_SIGNUP));
		}
		
		/**
			transfer confirm func.
		**/		
		[[eosio::action]]
		void transfer(name from, name to, asset quantity, string memo) {

			eosio_assert(quantity.is_valid(), ERROR_MSG_INVALID_ASSET);	

			// Recieve only
			if(from == _self) {
				return;
			}

			// [memo]
			// command-data
			string command;
			string data;
			/////////////////////////////////////////////////////////////////

			const size_t first_break = memo.find("-");

			// Just send
			if(first_break == string::npos){
				return;
			}

			command = memo.substr(0, first_break);
			data = memo.substr(first_break + 1);
			/////////////////////////////////////////////////////////////////

			if(command.empty()) {
				return;
			}

            // put-322
            // [command]-[user seq]
			if(command == COMMAND_NAME_PUT_TOKEN) {
				account_controller.charge_token(stoull(data, 0, 10), quantity);					
			}	
			
            // reward-1-321
            // [command]-[reward type]-[reward seq]
			else if (command == COMMAND_NAME_PUT_REWARD_POOL){
				const size_t second_break = data.find("-");
				uint32_t reward_type = stoull(data.substr(0, second_break), 0, 10);
				uint32_t reward_seq = stoull(data.substr(second_break + 1),0,10);

				reward_controller.put_reward_pool(reward_type,reward_seq,quantity);
			}
		}
		
		/**
			Set blockcahin id to user.
		**/
		[[eosio::action]]
		void setbcid(name blockchain_id, uint32_t user_seq){
			account_controller.set_blockchain_id(blockchain_id, user_seq);			
		}

		/**
			Return back token.
		**/
		[[eosio::action]]
		void withdraw(name requestor, uint32_t user_seq, asset value) {
			account_controller.withdraw_token(requestor, user_seq, value);	
			token_controller.send_token(_self, requestor, value, COMMENT_WITHDRAW_MEMO);	
		}
		
		/**
			Delete id.
		**/
		[[eosio::action]]
		void eraseid(uint32_t user_seq) {
			account_controller.delete_id(user_seq);
		}
		
		/**
			Set voting reward policy.
		**/
		[[eosio::action]]
		void setrwdpt(uint32_t reward_type, uint32_t reward_seq, asset quantity, uint8_t limit_type, uint32_t limit_info) {	
			reward_controller.set_reward_policy(reward_type, reward_seq, quantity, limit_type, limit_info);
		}
		
		/**
			Process voting reward policy.
		**/
		[[eosio::action]]
		void vtrewardproc() {
			reward_controller.do_vote_reward_process(reward_controller.get_rewards(REWARD_TYPE_VOTING)
													,globalvar_controller.get_globalvars(GLOBAL_VAR_ATTR_UNPROC_UP_VOTE_AMOUNT, GLOBAL_VAR_VALUE_UNPROC_UP_VOTE_AMOUNT)
													,globalvar_controller.get_globalvars(GLOBAL_VAR_ATTR_UNPROC_DOWN_VOTE_AMOUNT, GLOBAL_VAR_VALUE_UNPROC_DOWN_VOTE_AMOUNT));
			
			globalvar_controller.set_globalvars(GLOBAL_VAR_ATTR_UNPROC_UP_VOTE_AMOUNT, 0, 0);
			globalvar_controller.set_globalvars(GLOBAL_VAR_ATTR_UNPROC_DOWN_VOTE_AMOUNT, 0, 0);			
		}
		
		/**
			Erase reward.
		**/
		[[eosio::action]]
		void eraserwd(uint32_t seq) {
			reward_controller.delete_reward(seq);
		}
		
		/**
			Stake tokens to get voting power.
		**/
		[[eosio::action]]
		void stake(uint32_t user_seq, asset quantity) {
			token_controller.stake(user_seq, quantity);
		}

		/**
			Begine unstake. 
		**/
		[[eosio::action]]
		void beginunstake(uint32_t unstake_seq, uint32_t user_seq, asset value) {
			token_controller.begin_unstake(	unstake_seq, user_seq , value);
		}
		
		/**
			End unstake only if time is up.
		**/
		[[eosio::action]]
		void endunstake(uint32_t unstake_seq, uint32_t user_seq) {
			token_controller.end_unstake(unstake_seq, user_seq);
		}
		
		/**
			Charge voting power based on hour.
		**/
		[[eosio::action]]
		void chargevp(uint32_t user_seq) {
			token_controller.charge_voting_power(user_seq);				
		}
		
		/**
			Vote for the content.
		**/
		[[eosio::action]]
		void vote(uint32_t content_seq, uint32_t voter_seq, int64_t voting_power) {
			
			uint64_t spent_voting_power; 			

			if(voting_power > 0 ) {
				spent_voting_power = (uint64_t)voting_power;	
				globalvar_controller.set_globalvars(GLOBAL_VAR_ATTR_UNPROC_UP_VOTE_AMOUNT, spent_voting_power, 1);
			}
			else {
				spent_voting_power = (uint64_t)(voting_power * -1);	
				globalvar_controller.set_globalvars(GLOBAL_VAR_ATTR_UNPROC_DOWN_VOTE_AMOUNT, spent_voting_power, 1);				
			}
		
			token_controller.vote(content_seq, voter_seq, voting_power, spent_voting_power);
		}
		
		/**
			Donate tokens to writer.
		**/
		[[eosio::action]]	
		void donate(uint32_t content_seq, uint32_t donater_seq, asset quantity) {
			token_controller.donate(content_seq, donater_seq, quantity);
		}
		
		/**
			Register content.
		**/
		[[eosio::action]]
		void registerct(uint32_t ct_seq, uint32_t writer_seq) {
			content_controller.register_content(ct_seq, writer_seq, reward_controller.get_rewards(REWARD_TYPE_CONTENTS));
		}

		/**
			Delete content.
		**/
		[[eosio::action]]
		void erasect(uint32_t content_seq) {
			content_controller.delete_content(content_seq);
		}	

		/**
			Delete global var
		**/
		[[eosio::action]]
		void erasegv(uint32_t attr) {
			globalvar_controller.erase_gobal_var(attr);		
		}
				
};

#define EOSIO_DISPATCH_CUSTOM( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
	if(code == "todoskrtoken"_n.value && action == "transfer"_n.value) {\
            eosio::execute_action(name(receiver), name(code), &toodle::transfer);\
        }\
	else {\
			switch (action) {\
					EOSIO_DISPATCH_HELPER(TYPE, MEMBERS)\
				}\
	}\
   } \
} \

EOSIO_DISPATCH_CUSTOM(toodle, (signup)(transfer)(withdraw)(setbcid)(stake)(beginunstake)(endunstake)(vote)(chargevp)(registerct)(setrwdpt)(vtrewardproc)(donate)(eraseid)(erasect)(eraserwd))
