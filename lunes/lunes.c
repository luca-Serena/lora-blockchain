/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		For a general introduction to LUNES implmentation please see the 
		file: mig-agents.c

	Authors:
		First version by Gabriele D'Angelo <g.dangelo@unibo.it>

	############################################################################################### */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <ini.h>
#include <ts.h>
#include <rnd.h>
#include <gaia.h>
#include <rnd.h>
#include <values.h>
#include "utils.h"
#include "user_event_handlers.h"
#include "lunes.h"
#include "lunes_constants.h"
#include "entity_definition.h"


/* ************************************************************************ */
/* 		 L O C A L	V A R I A B L E S			    */
/* ************************************************************************ */

unsigned short	env_max_ttl = MAX_TTL;		// TTL of newly created messages


/* ************************************************************************ */
/* 			E X T E R N A L     V A R I A B L E S 	            */
/* ************************************************************************ */

extern hash_t			hash_table, *table;				/* Global hash table of simulated entities */
extern hash_t			sim_table, *stable;				/* Hash table of locally simulated entities */
extern double			simclock;						/* Time management, simulated time */
extern TSeed			Seed, *S;						/* Seed used for the random generator */
extern char				*TESTNAME;						/* Test name */
extern int	     		NSIMULATE;	 					/* Number of Interacting Agents (Simulated Entities) per LP */
extern int				NLP; 							/* Number of Logical Processes */
extern unsigned int		env_cache_size;					/* Cache size of each node */
extern float			env_end_clock;					/* End clock (simulated time) */
extern int 		  		env_block_frequency;  			/* A block is produced on average every env_block_frequency steps */
extern unsigned int		env_cache_size;					/* Cache size of each node */
extern int 		 		env_gateway_nodes;				/* Owners of gateways in the environment */
extern int 		 		env_sensor_nodes;				/* Owners of sensors in the environment */
extern int 				env_full_nodes;					/* Number of full nodes in the system*/
extern int  			env_clients;					/* Total clients making transactions*/
extern int              env_subs;						/* Number of subscriptions in the system */



int **subs_map;
int transactions_lost=0;
int transactions_total=0;
int transactions_validated=0;
int nextValidator=-1;
Transaction emptyTransaction = {.timestamp=0, .transactionID=0, .customer = -1, .sensor=-1, .gateway=-1};

/**********************SUPPORT FUNCTIONS*****************************************/

//Find a node a block in the local blockchain, given the ID
int findBlock (hash_node_t* node, long int id){
	int res = -1;
	for (int i = 0; i < BLOCKS_BUFFER_SIZE; i++){// node->data->numBlocks; i++){
		if (node->data->blocks[i].blockID == id){
			res = i;
			break;
		}
	}
	return res;
}


//TODO What do we have to check in order to check a block?
int checkBlock(hash_node_t* node, Block b){
	return 1;
}


int isBlockProducer (hash_node_t* node){
	int stakeCounter=0; //it counts all the stake of the full nodes at this moment
	int posCounter=0; //counter of al stakes of nodes with minor id. It's for simulating PoS 
	for (int i=0; i< env_full_nodes; i++){
		hash_node_t * otherNode = hash_lookup(table, i);
		stakeCounter += otherNode->data->coins;
		if (otherNode->data->key < node->data->key) {
			posCounter += otherNode->data->coins;
		}
	}
	int pseudorandom_for_pos = (int)simclock * 342 % stakeCounter;     //to decide which node validates the block
	if (stakeCounter == 0 && ((int) simclock / env_block_frequency) % env_full_nodes == node->data->key){
		return 1;
	} else if (stakeCounter > 0 && pseudorandom_for_pos >= posCounter && pseudorandom_for_pos < posCounter + node->data->coins){
		//printf ("__Chosen node__at %d__%d_____%d-%d-%d = %d____\n", (int)simclock, node->data->key, posCounter, posCounter + node->data->coins, stakeCounter, pseudorandom_for_pos); //DEBUG
		return 1;
	} else {
	    return 0;
	} 
}


//find if a block has already been produced 
int isThereBlockGivenHeight (hash_node_t* node, int heightRequired){
	if (heightRequired == 0){
		return 0;
	}
	int res = 0;
	for (int i = node->data->numBlocks; i >= 0; i--){
		if (node->data->blocks[i].height == heightRequired){
			res = 1;
			break;
		}
	}
	return res;
}

// Add block to the local blockchain of the node
 void add_block(hash_node_t *node, Block b){
 	if (node->data->numBlocks < BLOCKS_BUFFER_SIZE){
	 	node->data->blocks [node->data->numBlocks] = b;
	 	node->data->numBlocks++; 
	 	//remove all the confirmed transactions
	 	for (int i = 0; i< MAX_TRANSACTIONS_IN_BLOCKS; i++){
	 		for (int j=0; j<TRANSACTION_BUFFER_SIZE; j++){
	 			if (node->data->transactions[j].transactionID == b.transactions[i].transactionID){
	 				node->data->transactions[j] = emptyTransaction;
	 				break;
	 			}
	 		}
	 	}
	} else {
		printf("FATAL ERROR AT %d BUFFER FOR BLOCKS OF THE NODE %d IS FULL. TRY TO INCREASE IT\n", (int)simclock, node->data->key);
		fflush(stdout);
		exit(-1);
	}
 }


// Add request to the local requests list of the node
 void add_transaction(hash_node_t *node, Transaction tr){
 	int found = 0;
 	for (int i =0; i<TRANSACTION_BUFFER_SIZE; i++){
 		if (node->data->transactions[i].transactionID <= 0){
 			node->data->transactions[i] = tr;
 			found = 1;
 			break;
 		}
 	}
 	if (found == 0){
 		transactions_lost++;
 		/*printf("AT %d BUFFER FOR TRANSACTIONS IS FULL FOR NODE %d. TRY TO INCREASE IT. NOW IT IS %d\n", (int)simclock, node->data->key, TRANSACTION_BUFFER_SIZE);
 		fflush(stdout);
		exit(-1);*/
 	}
}


void print_block(Block *b){
	printf(" Block with ID %ld at height %d created by node %d at %d ", b->blockID, b->height, b->blockMaker, b->timestamp);
	int counter = 0;
	for (int i = 0; i< MAX_TRANSACTIONS_IN_BLOCKS; i++){
		if (b->transactions[i].transactionID > 0){
			counter ++;
			//DEBUG
			//printf("tx: %ld   ", b->transactions[i].transactionID);
			//printf("tx: %d %d %d %d -- ", b->transactions[i].sensor, b->transactions[i].gateway, b->transactions[i].provider, b->transactions[i].customer);
		}
	}
	printf("with %d transactions\n", counter);
}



/*

void print_blockchain (hash_node_t* node){
	int last_index = lastBlock(node);
	if (last_index < 0) {
		return;
	}
	Block last = node->data->blocks[last_index];
	fprintf(stdout, "*****Subnet %d *******\n", node->data->subnetID);
	print_block(&last);
	while (last.previousBlockID > 0 ){
		last_index = findBlock (node, last.previousBlockID);
		last = node->data->blocks[last_index];
		print_block(&last);
	}
	fprintf(stdout, " **\n");
}
*/
//*************************************FUNCTIONS TO MANAGE THE CACHE*****************************************************

//Chose the oldest element in the cache (the oldest is probably the least useful)
int cache_choose_oldest (hash_node_t *node){
    int oldest = env_end_clock;
    int res = 0;
    for (int i=0; i< CACHE_SIZE; i++){
        if (node->data->cache[i].timestamp <=0){      //if (&node->data->cache[i] == NULL){
            res = i;
            break;
        } else if (node->data->cache[i].timestamp < oldest){
            oldest = node->data->cache[i].timestamp;
            res = i;
        }
    }
    return res;
}

//add a new element in the cache (if there is no space available, replace this with the oldest)
void add_into_cache (hash_node_t *node, cache_element elem){
    int newIndex = cache_choose_oldest(node);
    node->data->cache[newIndex] = elem;
}

//check if the received element is in the cache
int is_in_cache (hash_node_t *node, long int elemId){
    int res = 0;
    for (int i = 0; i< CACHE_SIZE; i++){
        if (node->data->cache[i].id == elemId){
            res = 1;
            break;
        }
    }
    return res;
}

//Return how many elemements are actually put in the cache
int count_cacheSize (hash_node_t *node){
    int res = 0;
    for (int i = 0; i < CACHE_SIZE; i++){
        if (node->data->cache[i].id != -1){
            res++;
        }
    }
    return res;
}


//*************************************INITIALIZATION FUNCTIONS*****************************************************


void generate_subs (){
	subs_map = (int**)malloc(env_subs * sizeof(int*));
    if (subs_map == NULL) {
        printf("Memory allocation for rows failed. Exiting...\n");
        exit(-1);
    }

    for (int i = 0; i < env_subs; i++) {
        subs_map[i] = (int*)malloc(2 * sizeof(int));
        if (subs_map[i] == NULL) {
            printf("Memory allocation for columns in row %d failed. Exiting...\n", i);
            exit(-1);
        }
    }

	for (int i =0; i < env_subs; i++){
		subs_map [i][0] = env_full_nodes + env_gateway_nodes + env_sensor_nodes + (i % env_clients); //client that is subscribed
		subs_map [i][1] = env_full_nodes + env_gateway_nodes + (i % env_sensor_nodes); //sensor to which the client is subscribed
	}
}



//create neighborhood links. All the nodes are connected with the other nodes belonging to the same subnet (in different data center)
void createLinks (hash_node_t *node){
	for (int i = 0; i < NSIMULATE; i++){
		hash_node_t	* otherNode = hash_lookup(table, i);
		if (otherNode->data->type == 'P' || node->data->type == 'P'){
			execute_link (simclock + FLIGHT_TIME, node, otherNode);
		}
	}
}



//***************************************SENDERS TO NEIGHBORS VIA BROADCAST***********************************************************

void lunes_send_block_to_neighbors (hash_node_t *node, Block b) {

	// Iterator to scan the whole state hashtable of neighbors
	GHashTableIter		iter;
	gpointer		key, destination;
	// All neighbors
	g_hash_table_iter_init (&iter, node->data->state);

	while (g_hash_table_iter_next (&iter, &key, &destination)) {
		hash_node_t* destNode = hash_lookup(table, *(unsigned int *)destination);
		int nodes_flight_time = 1;
		execute_block (simclock + nodes_flight_time, node, destNode, env_max_ttl, b, simclock, node->data->key);
	}
}


void lunes_send_transaction_to_neighbors (hash_node_t *node, Transaction tr) {
	// Iterator to scan the whole state hashtable of neighbors
	GHashTableIter		iter;
	gpointer		key, destination;
	// All neighbors
	g_hash_table_iter_init (&iter, node->data->state);

	while (g_hash_table_iter_next (&iter, &key, &destination)) {
		hash_node_t* destNode = hash_lookup(table, *(unsigned int *)destination);
		int nodes_flight_time = 1;
		execute_transaction (simclock + nodes_flight_time, node, destNode, env_max_ttl, tr, simclock, node->data->key);
	}
}


//****************************************************GENERATE TRANSACTIONS MANAGEMENT*************************************************************************

//generate the transactions: all customers subscribed to a certain flow of data will pay a reward to the the gateway and the sensor
void generate_transaction (hash_node_t *node, int sensor, int gw, int ts, int provider){
	if  ((provider < env_full_nodes && gw >= env_full_nodes && gw < env_full_nodes + env_gateway_nodes && sensor >= env_full_nodes + env_gateway_nodes &&
		sensor < env_full_nodes + env_gateway_nodes + env_sensor_nodes) == 0){
		perror("Error in the assignment of the IDs for sensors / gateways / providers\n");
		fprintf(stdout, "____________%d %d %d\n", sensor, gw, provider);
	    exit(-1);
	}
	for (int i=0; i < env_subs; i++){
		if (subs_map [i][1] == sensor){  // a transaction for each couple of data-subscriber
			int sensor_provider =  sensor % env_full_nodes;      // temporary strategy
			Transaction t = {.timestamp = ts, .gateway = gw, .sensor = sensor, .customer = subs_map[i][0], .transactionID =  RND_Interval (S, 0, 1000000000000), .provider=sensor_provider};
			cache_element c = {.id = t.transactionID, .timestamp = (int)simclock};
			add_into_cache(node, c);
			add_transaction(node, t);
			transactions_total++;
			lunes_send_transaction_to_neighbors (node, t);
		}
	}
}


// read the flow of data from the temporary file
void read_transactions(hash_node_t *node){
	char line[256];
	char transmissionsFileName [17] = "transmissions.txt";
	char flagFile [15];
	snprintf(flagFile, sizeof(flagFile), "%s%d%s", "step", (int)(simclock), ".txt");
	FILE *fp = fopen (transmissionsFileName, "r");
	if (fp != NULL){
     	while (fgets(line, sizeof(line), fp) != NULL) {     // Initialize variables to store the three fields
	    	char field1[64], field2[64], field3[64]; 
		    // Use sscanf to read the fields
		    if (sscanf(line, "%s %s %s", field1, field2, field3) == 3) {
		    	memmove(field1, field1 + 3, strlen(field1) - 2);
				int sensor = atoi(field1) % env_sensor_nodes + env_full_nodes + env_gateway_nodes;
				int gateway = atoi (field2) + env_full_nodes;
				int provider = atoi (field3);
				//fprintf(stdout, "%d %d %d___\n", sensor, gateway, timestamp);
				generate_transaction (node, sensor, gateway, (int)simclock, provider);
		    } else { // Handle cases where a line does not have three fields
		        printf("Invalid line: %s\n", line);
		        exit (-1);
		    }
	    }
	    fclose(fp);
    }

    if (remove(flagFile) != 0) {
        fprintf(stderr, "Error deleting the file %s\n", flagFile);
        //exit(-1);
    }  
}



void lunes_print_stats(){
	printf ("Number of messages lost per node because the buffer for transactions is full: %d\n", transactions_lost/env_full_nodes);
	printf ("Transaction validated: %d out of %d\n", transactions_validated, transactions_total);
}

/* ************************************************************************ */
/* 	L U N E S     U S E R    L E V E L     H A N D L E R S		    */
/* ************************************************************************ */


void lunes_initialize_agents (hash_node_t *node) {

	if (node->data->key < env_full_nodes){  //full nodes - nodes by providers
		node->data->type = 'P';
		node->data->coins = 10;
	} else if (node->data->key < env_full_nodes + env_gateway_nodes) {
		node->data->type = 'G';
	} else if (node->data->key < env_full_nodes + env_gateway_nodes + env_sensor_nodes) {
		node->data->type = 'S';
	} else {
		node->data->type = 'C';
	}
}


/****************************************************************************
	LUNES_CONTROL: node activity for the current timestep
*/

void lunes_user_control_handler (hash_node_t *node) {

	//read transactions and decide who is the next validator. Decision of next validator performed here to avoid conflicts with PoS
	if (node->data->type == 'P' && (int)simclock % env_block_frequency == 0 && isBlockProducer(node)){
		read_transactions(node);
		nextValidator = node->data->key;
	}

	//produce the block if the node is the validator
	if (node->data->key == nextValidator && (int)simclock % env_block_frequency == 5) {
		int newHeight=0, prevBlockID = -1;
		if (node->data->numBlocks > 0){
			prevBlockID = node->data->blocks[node->data->numBlocks - 1].blockID;
			newHeight = node->data->blocks[node->data->numBlocks - 1].height + 1;
		}
		int counter=0;
		Block b = {.blockID=RND_Interval (S, 0, 1000000000000), .timestamp = (int)simclock, .blockMaker = node->data->key,
					.previousBlockID = prevBlockID, .height = newHeight, .confirmations = 1};

		for (int i =0; i< TRANSACTION_BUFFER_SIZE; i++){
			if (node->data->transactions[i].transactionID > 0){
				b.transactions[counter] = node->data->transactions[i];
				hash_lookup(table, b.transactions[counter].gateway)->data->coins++;   //reward the gateway
				hash_lookup(table, b.transactions[counter].sensor)->data->coins++;    //reward the sensor
				hash_lookup(table, b.transactions[counter].provider)->data->coins++;    //reward the sensor
				//printf ("rewarding %d and %d at %d\n", b.transactions[counter].sensor, b.transactions[counter].gateway, (int)simclock);  //DEBUG
				counter++;
				node->data->transactions[i] = emptyTransaction;
				if (counter == MAX_TRANSACTIONS_IN_BLOCKS){
					printf("max reached\n");
					break;
				}
			}
		}
		node->data->numBlocks++;
		transactions_validated += counter;
		add_block(node, b);
		cache_element c = {.id = b.blockID, .timestamp = (int)simclock};
		add_into_cache (node, c);
		lunes_send_block_to_neighbors (node, b);
		print_block(&b);
	}
}



/****************************************************************************
	LUNES_REGISTER: a new SE (in this LP) has been created, LUNES needs to
		initialize some data structures
*/
void lunes_user_register_event_handler (hash_node_t *node) {
	float	threshold;	// Tmp, probabilistic evaluation
	// Only a given percentage of nodes generates new messages
	threshold = RND_Interval (S, (double)0, (double)100);

	if ( threshold <= PERC_GENERATORS ) {
		// Initialization of the time for the generation of new messages
		node->data->s_state.time_of_next_message = simclock + (RND_Exponential(S, 1) * MEAN_NEW_MESSAGE);
	} else	// This node will NOT generate new messages (it is a forwarder) 
		node->data->s_state.time_of_next_message = -1;		// This node will not generate messages
}

/****************************************************************************
	LUNES_TRANSACTION: what happens in LUNES when a node receives a TRANSACTION message?
*/
void lunes_user_transaction_event_handler (hash_node_t *node, int forwarder, Msg *msg) {
	Transaction tr = msg->transaction.transaction;
	if (is_in_cache(node, tr.transactionID) == 0){ //if it not in the cache
		cache_element c = {.id = tr.transactionID, .timestamp = (int)simclock};
		add_into_cache (node, c);
		add_transaction(node, tr);
 	}
} 

/****************************************************************************
	LUNES_BLOCK: what happens in LUNES when a node receives a BLOCK message?
*/
void lunes_user_block_event_handler (hash_node_t *node, int forwarder, Msg *msg) {
	Block b = msg->block.block;
	if (is_in_cache(node, b.blockID) == 0){ //if it not in the cache
		cache_element c = {.id = msg->block.block.blockID, .timestamp = (int)simclock};
		add_into_cache (node, c);
		if (node->data->type == 'P' && checkBlock(node, b) == 1){
			//b.confirmations++;
			lunes_send_block_to_neighbors (node, b);
		}	
	}
	if (/*b.confirmations > env_quorum && */isThereBlockGivenHeight(node, b.height) == 0){
		add_block (node, b);
	}
}


void lunes_user_confirmation_event_handler (hash_node_t *node, int forwarder, Msg *msg){
}