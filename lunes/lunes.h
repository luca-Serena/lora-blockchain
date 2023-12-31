/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		-	Function prototypes

	Authors:
		First version by Gabriele D'Angelo <g.dangelo@unibo.it>

	############################################################################################### */

#ifndef __LUNES_H
#define __LUNES_H

#include "utils.h"

// LUNES handlers
void	lunes_user_transaction_event_handler ( hash_node_t *, int, Msg * );
void	lunes_user_block_event_handler ( hash_node_t *, int, Msg * );
void	lunes_user_confirmation_event_handler ( hash_node_t *, int, Msg * );
void	lunes_user_register_event_handler ( hash_node_t * );
void 	lunes_initialize_agents ( hash_node_t * );
void	lunes_user_control_handler ( hash_node_t * );
void	createLinks ( hash_node_t * );
void    generate_subs ();
void    lunes_print_blockchain (hash_node_t *);

// Support functions
void 	lunes_load_entities_location ();
void    lunes_print_stats();

#endif /* __LUNES_H */

