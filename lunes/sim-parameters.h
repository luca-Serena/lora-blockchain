/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		-	Model level "hard coded" simulation parameters
		-	Uncomment DEBUG defines for very verbose output

	Authors:
		First version by Gabriele D'Angelo <g.dangelo@unibo.it>

	############################################################################################### */


// NOTE:
//	some of the following values can be superseded by environmental variables,
//	see the user_environment_handler() in the file: user_event_handler.c

/****************************** DEBUG *****************************************/
// Uncomment to activate debug
//
//	general debug
//#define DEBUG
//
//	traces every ping message (very verbose)
//#define PINGDEBUG
//
//	traces every cache access and answer
//#define CACHEDEBUG
//
//	traces every packet drop due to TTL
//#define TTLDEBUG
//
//	traces the actions of the adaptive gossip dissemination protocol
//#define AG_DEBUG
//
//	traces the actions of free riding nodes
//#define FREERIDINGDEBUG
//
//	if not defined the tracing of dissemination protocol messages is disabled
//#define TRACE_DISSEMINATION

//#define MIGRATION_INFO




/**************************** MODEL ****************************************/
// Simulation length (final clock value), default value
#define END_CLOCK			100.0

// This timestep is chosen to build the aggregation structure
#define BUILDING_STEP			3

// At this timestep the aggregation is completed and the nodes' start pinging each other
#define EXECUTION_STEP			5

// Number of timestep required by ping messages to receive the destination node
//	WARNING: due to synchronization constraints The FLIGHT_TIME has to be bigger 
//		than the timestep size
#define FLIGHT_TIME			1.0

#define CACHE_SIZE			100

#define BLOCKS_BUFFER_SIZE		2000

#define TRANSACTION_BUFFER_SIZE	500

#define MAX_TRANSACTIONS_IN_BLOCKS	20

#define MAX_CROSS_SUBNETS 		5

/************************ SIMULATOR  LIMITS ********************************/

// Max number of records that can be inserted in a single ping message
#define	MAX_PING_DYNAMIC_RECORDS	0

// Max number of records that can be inserted in a single migration message
#define	MAX_MIGRATION_DYNAMIC_RECORDS	1000

// Buffer size for incoming messages
//	obviously the buffer needs to be so large to contain all kind of messages
//	(e.g. ping and migration messages)
#define BUFFER_SIZE			1024*1024


