/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		-	In this file is defined the state of the simulated entities

	Authors:
		First version by Gabriele D'Angelo <g.dangelo@unibo.it>

	############################################################################################### */

#ifndef __ENTITY_DEFINITION_H
#define __ENTITY_DEFINITION_H

#include "lunes_constants.h"

typedef struct Transaction{
	int timestamp;                        		        //time-step where the transaction has been originated
	long int transactionID;
	int customer;						        // id of the node sending a certain amount of coins
	int gateway;						        // id of the gateway that has transmitted the data and will receive the reward
	int sensor;						  	// id of the sensor the collected the data and will receive the reward
	int provider;							
}Transaction;


typedef struct Block{
	long int blockID;                                            // ID of the current block
	int timestamp;
	long int previousBlockID;					// ID of the previous block
	int height;							// height -1 block before
	int blockMaker;						// id of the validator
	int confirmations;						// number of nodes that confirmed the validity of the block
	Transaction transactions [MAX_TRANSACTIONS_IN_BLOCKS] ;
}Block;

/*---- E N T I T I E S    D E F I N I T I O N ---------------------------------*/

typedef struct v_e {
	unsigned int	value;					// Value
} value_element;

// Records composing the local state (dynamic part) of each SE
// NOTE: no duplicated keys are allowed
struct state_element {
	unsigned int	key;					// Key
	value_element	elements;				// Value
};

typedef struct cache_element{
    long int id;
    unsigned int timestamp;
} cache_element;

// Static part of the SE state
typedef struct static_data_t {
	char			changed;			// ON if there has been a state change in the last
	float			time_of_next_message;		// Timestep in which the next new message will be created and sent
} static_data_t;

// SE state definition
typedef struct hash_data_t {
	char 			type;						// Client, provider ,sensor owner or gateways
	int			key;						// SE identifier
	int 			lp;						// Logical Process ID (that is the SE container)
	static_data_t		s_state;					// Static part of the SE local state
	GHashTable*		state;						// Local state as an hash table (glib) (dynamic part)
	Block			blocks[BLOCKS_BUFFER_SIZE];			// Blocks in subnet
	int			numBlocks;
	int 			coins;						// Amount of coins owned by a certain node
	int 			confirmations;					// number of confirmations about the validity of the block
	Transaction            transactions[TRANSACTION_BUFFER_SIZE]; 	// Transactions waiting to be approved
        cache_element          cache [CACHE_SIZE];				// cache
} hash_data_t;

#endif /* __ENTITY_DEFINITION_H */
