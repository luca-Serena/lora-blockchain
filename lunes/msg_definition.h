/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		-	In this file are defined the message types and their structures

	Authors:
		First version by Gabriele D'Angelo <g.dangelo@unibo.it>

	############################################################################################### */

#ifndef __MESSAGE_DEFINITION_H
#define __MESSAGE_DEFINITION_H

#include "entity_definition.h"

/*---- M E S S A G E S    D E F I N I T I O N ---------------------------------*/
typedef struct _ping_msg		PingMsg;		// Interactions among messages
typedef struct _link_msg		LinkMsg;		// Network constructions
typedef struct _transaction_msg	TrnMsg;		// Transaction forwarding
typedef struct _block_msg 		BlockMsg; 		// Block forwarding
typedef struct _confirmation_msg	ConfirmationMsg;	



// Model messages definition
typedef struct _migr_msg		MigrMsg;		// Migration message
typedef union   msg			Msg;

// General note:
//	each type of message is composed of a static and a dynamic part
//	-	the static part contains a pre-defined set of variables, and the size
//		of the dynamic part (as number of records)
//	-	a dynamic part that is composed of a sequence of records
// **********************************************
// PING MESSAGES
// **********************************************
// Record definition for dynamic part of ping messages (THIS DYNAMIC PART IS NOT USED IN LUNES)
struct _ping_record {
	unsigned int	key;
	unsigned int	value;
};
//
// Static part of ping messages
struct _ping_static_part {
	char		type;					// Message type
	float		timestamp;				// Timestep of creation (of the message)
	unsigned short	ttl;					// Time-To-Live
	unsigned int	msgvalue;				// Message Identifier
	unsigned int	creator;				// ID of the original sender of the message
	unsigned int	dyn_records;				// Number of records in the dynamic part of the message
};
//
// Dynamic part of ping messages
struct _ping_dynamic_part {
	struct _ping_record	records[MAX_PING_DYNAMIC_RECORDS];		// Adjuntive records
};
//
// Ping message
struct _ping_msg {
	struct	_ping_static_part		ping_static;			// Static part
	struct	_ping_dynamic_part		ping_dynamic;			// Dynamic part
};


struct _transaction_msg  {
	char		type;							// Message type
	float		timestamp;						// Timestep of creation (of the message)
	unsigned short	ttl;							// Time-To-Live
	Transaction 	transaction;						// Transaction to be sent
	unsigned int	creator;						// ID of the original sender of the message
	long int id;
};

struct _block_msg {
	char		type;
	float		timestamp;						// Timestep of creation (of the message)
	unsigned short	ttl;							// Time-To-Live
	Block	 	block;							// Block to be sent
	unsigned int	creator;						// ID of the original sender of the message
	long int 	blockID;
};


struct _confirmation_msg{
	char		type;
	float		timestamp;						// Timestep of creation (of the message)
	unsigned short	ttl;							// Time-To-Live
	long int	blockID;						// Block to be finalized
	int		creator;						// ID of the original sender of the message
};



// **********************************************
// LINK MESSAGES
// **********************************************
// Record definition for dynamic part of link messages
struct _link_record {
	unsigned int	key;
	unsigned int	value;
};
//
// Static part of link messages
struct _link_static_part {
	char		type;							// Message type
	unsigned int	dyn_records;						// Number of records in the dynamic part of the message
};
//
// Dynamic part of link messages
struct _link_dynamic_part {
	struct _link_record	records[0];					// It is an array of records
};
//
// Link message
struct _link_msg {
	struct	_link_static_part		link_static;			// Static part
	struct	_link_dynamic_part		link_dynamic;			// Dynamic part
};

//

/////////////////////////////////////////////////
// Union structure for all types of messages
union msg {
	char			type;
	LinkMsg		link;
    	PingMsg		ping;
    	TrnMsg			transaction;
	BlockMsg	        block;
	ConfirmationMsg	confirmation;
};
/*---------------------------------------------------------------------------*/

#endif /* __MESSAGE_DEFINITION_H */
