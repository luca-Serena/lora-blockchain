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

