/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		Model level parameters for LUNES

	Authors:
		First version by Gabriele D'Angelo <g.dangelo@unibo.it>

	############################################################################################### */

#ifndef __LUNES_CONSTANTS_H
#define __LUNES_CONSTANTS_H

// 	General parameters
#define MAX_CACHE_SIZE				512				// MAX cache size (in each node)
#define MAX_TTL					10				// TTL of new messages, standard value
#define	MEAN_NEW_MESSAGE 			30				// Generation of new messages: exponential distribution, mean value
#define PERC_GENERATORS				100.00				// Percentage of nodes that generate new messages

//	Dissemination protocols
#define BROADCAST			0	// Probabilistic broadcast
#define GOSSIP_FIXED_PROB		1	// Fixed probability
#define GOSSIP_FIXED_FANOUT		2	// Fixed fanout (NOT IMPLEMENTED IN THIS VERSION)



#endif /* __LUNES_CONSTANTS_H */


// NOTE:
//	some of the following values can be superseded by environmental variables,
//	see the user_environment_handler() in the file: user_event_handler.c


/**************************** MODEL ****************************************/
// Simulation length (final clock value), default value
#define END_CLOCK			1000

// This timestep is chosen to build the aggregation structure
#define BUILDING_STEP			3

// At this timestep the aggregation is completed and the nodes' start pinging each other
#define EXECUTION_STEP			5

// Number of timestep required by ping messages to receive the destination node
//	WARNING: due to synchronization constraints The FLIGHT_TIME has to be bigger 
//		than the timestep size
#define FLIGHT_TIME			1.0

#define CACHE_SIZE			1000

#define BLOCKS_BUFFER_SIZE		2000

#define TRANSACTION_BUFFER_SIZE	1000

#define MAX_TRANSACTIONS_IN_BLOCKS	200

/************************ SIMULATOR  LIMITS ********************************/

// Max number of records that can be inserted in a single migration message
#define	MAX_MIGRATION_DYNAMIC_RECORDS	1000

// Buffer size for incoming messages
//	obviously the buffer needs to be so large to contain all kind of messages
//	(e.g. ping and migration messages)
#define BUFFER_SIZE			1024*1024


