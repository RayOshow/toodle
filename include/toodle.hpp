#pragma once

#define TEST_MODE_ON 1

// Contract name
#define CONTRACT_NAME "toodle"

// Token Contract name
#define TOKEN_CONTRACT "todoskrtoken"_n
#define REWARD_ACCOUNT "toodlereward"_n

// Token transfer command
#define COMMAND_NAME_PUT_TOKEN "put"
#define COMMAND_NAME_PUT_REWARD_POOL "reward"

// Main token info
#define MAIN_TOKEN_SYMBOL "TRP"
#define MAIN_TOKEN_SYMBOL_DECIMAL 4
#define MAIN_TOKEN_SYMBOL_DECIMAL_N 10000

// Stake info
#if TEST_MODE_ON
	#define UNSTAKING_SEC (10*1*1)
	#define MIN_CHARGE_SEC (10*1*2)
	#define MAX_CHARGE_SEC (60*1*2)
#else
	#define UNSTAKING_SEC (60*60*72)
	#define MIN_CHARGE_SEC (60*60*2)
	#define MAX_CHARGE_SEC (60*60*24)
#endif

	
// global variables 
#define GLOBAL_VAR_ATTR_UNPROC_UP_VOTE_AMOUNT 1
#define GLOBAL_VAR_ATTR_UNPROC_DOWN_VOTE_AMOUNT 2

#define GLOBAL_VAR_VALUE_UNPROC_UP_VOTE_AMOUNT 0
#define GLOBAL_VAR_VALUE_UNPROC_DOWN_VOTE_AMOUNT 0

// COMMENT
#define COMMENT_WITHDRAW_MEMO "Reward system withdraw token success!!"


// Reward type
#define REWARD_TYPE_SIGNUP 1
#define REWARD_TYPE_CONTENTS 2
#define REWARD_TYPE_VOTING 3

// Reward status
#define REWARD_STATUS_RESERVE 0
#define REWARD_STATUS_USE 1
#define REWARD_STATUS_CLOSE 2

// Reward limiit type
#define REWARD_LIMIT_TYPE1_TIME 1
#define REWARD_LIMIT_TYPE2_CNT 2

// Reward ratio between writer and voter
#define REWARD_RATIO_WRITER 75 / 100

// Erorr code
#define ERROR_MSG_INVALID_ASSET "{\"CODE\":\"0001\":,\"MSG\":\"ERROR_MSG_INVALID_ASSET\"}"
#define ERROR_MSG_SYMBOL_NOT_EXIST "{\"CODE\":\"0002\",\"MSG\":\"ERROR_MSG_SYMBOL_NOT_EXIST\"}"
#define ERROR_MSG_ID_NOT_EXIST "{\"CODE\":\"0003\",\"MSG\":\"ERROR_MSG_ID_NOT_EXIST\"}"
#define ERROR_MSG_ID_ALREADY_EXIST "{\"CODE\":\"0004\",\"MSG\":\"ERROR_MSG_ID_ALREADY_EXIST\"}"
#define ERROR_MSG_ID_NOT_MATCH "{\"CODE\":\"0005\",\"MSG\":\"ERROR_MSG_ID_NOT_MATCH\"}"
#define ERROR_MSG_ID_NOT_ENOUGH_TOKEN "{\"CODE\":\"0006\",\"MSG\":\"ERROR_MSG_ID_NOT_ENOUGH_TOKEN\"}"
#define ERROR_MSG_STAKE_NO_INFO "{\"CODE\":\"0007\",\"MSG\":\"ERROR_MSG_STAKE_NO_INFO\"}"
#define ERROR_MSG_STAKE_NOT_ENOUGH_TIME "{\"CODE\":\"0008\",\"MSG\":\"ERROR_MSG_STAKE_NOT_ENOUGH_TIME\"}"
#define ERROR_MSG_UNSTAKE_ALREADY "{\"CODE\":\"0009\",\"MSG\":\"ERROR_MSG_UNSTAKE_ALREADY\"}"
#define ERROR_MSG_CONTENTS_ALREADY_EXIST "{\"CODE\":\"0010\",\"MSG\":\"ERROR_MSG_CONTENTS_ALREADY_EXIST\"}"
#define ERROR_MSG_CONTENTS_NOT_EXIST "{\"CODE\":\"0011\",\"MSG\":\"ERROR_MSG_CONTENTS_NOT_EXIST\"}"
#define ERROR_MSG_CONTENTS_NOT_ENOUGH_VOTING_POWER "{\"CODE\":\"0012\",\"MSG\":\"ERROR_MSG_CONTENTS_NOT_ENOUGH_VOTING_POWER\"}"
#define ERROR_MSG_CONTENTS_VOTER_IS_WRITER "{\"CODE\":\"0013\",\"MSG\":\"ERROR_MSG_CONTENTS_VOTER_IS_WRITER\"}"
#define ERROR_MSG_INVALID_REWARD_CONTRACT "{\"CODE\":\"0014\",\"MSG\":\"ERROR_MSG_INVALID_REWARD_CONTRACT\"}"
#define ERROR_MSG_INVALID_TOKEN_CONTRACT "{\"CODE\":\"0015\",\"MSG\":\"ERROR_MSG_INVALID_TOKEN_CONTRACT\"}"
#define ERROR_MSG_INVALID_REWARD_AMOUNT "{\"CODE\":\"0016\",\"MSG\":\"ERROR_MSG_INVALID_REWARD_AMOUNT\"}"
#define ERROR_MSG_REWARD_NOT_EXIST "{\"CODE\":\"0017\",\"MSG\":\"ERROR_MSG_REWARD_NOT_EXIST\"}"
#define ERROR_MSG_REWARD_NOT_ENOUGH "{\"CODE\":\"0018\",\"MSG\":\"ERROR_MSG_REWARD_NOT_ENOUGH\"}"
#define ERROR_MSG_UNPROC_VOTE_NOT_ENOUGH "{\"CODE\":\"0019\",\"MSG\":\"ERROR_MSG_UNPROC_VOTE_NOT_ENOUGH\"}"
#define ERROR_MSG_VOTE_CHARGE_TIME_NOT_ENOUGH "{\"CODE\":\"0020\",\"MSG\":\"ERROR_MSG_VOTE_CHARGE_TIME_NOT_ENOUGH\"}"
