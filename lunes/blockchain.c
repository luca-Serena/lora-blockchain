/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
#include "utils.h"
#include "user_event_handlers.h"

/*-------- G L O B A L     V A R I A B L E S --------------------------------*/

int      	NSIMULATE, 		// Number of Interacting Agents (Simulated Entities) per LP
		NLP, 			// Number of Logical Processes
		LPID,			// Identification number of the local Logical Process 
		local_pid;		// Process Identifier (PID)

// SImulation MAnager (SIMA) information and localhost identifier
static char	LP_HOST[64];		// Local hostname
static char	SIMA_HOST[64];		// SIMA execution host (fully qualified domain)
static int	SIMA_PORT;		// SIMA execution port number

// Time management variables
double		step,			// Size of each timestep (expressed in time-units)
		simclock = 0.0;		// Simulated time
static int	end_reached = 0;	// Control variable, false if the run is not finished

// A single LP is responsible to show the runtime statistics
//	by default the first started LP is responsible for this task
static int	LP_STAT = 0;

// Seed used for the random generator
TSeed		Seed, *S=&Seed;
/*---------------------------------------------------------------------------*/

// File descriptors: 
//	lcr_fp: (output) -> local communication ratio evaluation
//	finished: (output) -> it is created when the run is finished (used for scripts management)
//
FILE	*finished_fp;

// Output directory (for the trace files)
char	*TESTNAME;

// Simulation control (from environment variables, used by batch scripts)
unsigned int	env_migration;					// Migration state
float		env_migration_factor;			// Migration factor
unsigned int	env_load;						// Load balancing
float		env_end_clock = END_CLOCK;		// End clock (simulated time)
unsigned int	env_migration;				// Migration state
unsigned int	env_cache_size;				// Cache size of each node
int 		env_gateway_nodes;			/* Owners of gateways in the environment */
int 		env_sensor_nodes;			/* Owners of sensors in the environment */
int 		env_full_nodes;				/* Number of full nodes in the system*/
int  		env_clients;				/* Total clients making transactions*/
int 		env_block_frequency;  			/* A block is produced on average every env_block_frequency steps (one for each subnet)*/
int             env_subs;				/* Number of subscriptions in the system */



/* ************************************************************************ */
/* 			            Hash Tables		      	            */
/* ************************************************************************ */

hash_t		hash_table,  *table=&hash_table;		/* Global hash table, contains ALL the simulated entities */
hash_t		sim_table,   *stable=&sim_table;		/* Local hash table, contains only the locally managed entities */
/*---------------------------------------------------------------------------*/


/* ************************************************************************ */
/* 			   Migrating Objects List			    */
/* ************************************************************************ */

// List containing the objects (SE) that have to migrate at the end of the 
//	current timestep
static	se_list		migr_list,	
	*mlist 	= 	&migr_list;
/*---------------------------------------------------------------------------*/


/* ************************************************************************* 
 		      M O D E L    D E F I N I T I O N	
	NOTE:	in the following there is only a part of the model definition, the
		most part of it is implemented in the user level,
		see: user_event_handlers.c and lunes.c
**************************************************************************** */



//      Computation and Interactions generation: called at each timestep it will provide the model behavior
static void     Generate_Computation_and_Interactions (int total_SE) {

        // Call the appropriate user event handler
        user_control_handler();
}



//	SEs initial generation: called once when global variables have been initialized.
static void Generate (int count) {

	int	i;
	// The local Simulated Entities are registered using the appropriate GAIA API
	for ( i = 0; i < count; i++ ) {

		// In this case every entity can be migrated
		GAIA_Register ( MIGRABLE );

		// NOTE:	the internal state of entities is initialized in the register_event_handler()
		//		see in the following of this source file
	}
}


/*---------------------------------------------------------------------------*/


/* ************************************************************************ */
/* 			 E V E N T   H A N D L E R S			    */
/* ************************************************************************ */


/*
	Upon arrival of a model level event, firstly we have to validate it
	and only in the following the appropriate handler will be called
*/
struct hash_node_t*	validation_model_events (int id, int to, Msg *msg) {

	struct hash_node_t	*node;


	// The receiver has to be a locally manager Simulated Entity, let's check!
	if ( ! (node = hash_lookup ( stable, to ) ) )  {

		// The receiver is not managed by this LP, it is really a fatal error
		fprintf(stdout, "%12.2f node: FATAL ERROR, [%5d] is NOT in this LP!\n", simclock, to);
		fflush(stdout);
		exit(-1);
	}  else	return( node );
}


/*
 	A new SE has been created, we have to insert it into the global 
	and local hashtables, the correct key to use is the sender's ID
*/
static void register_event_handler (int id, int lp) {

 	hash_node_t 		*node;
	
	// In every case the new node has to be inserted in the global hash table
	//	containing all the Simulated Entities
	node = hash_insert ( GSE, table, NULL, id, lp );
	
	if ( node ) {
		node->data->s_state.changed = YES;

		// If the SMH is local then it has to be inserted also in the local
		//	hashtable and some extra management is required				
		if ( lp == LPID ) {

			// Call the appropriate user event handler
			user_register_event_handler( node, id );

			// Inserting it in the table of local SEs
			if ( ! hash_insert( LSE, stable, node->data, node->data->key, LPID ) ) {
				// Unable to allocate memory for local SEs 
				fprintf(stdout, "%12.2f node: FATAL ERROR, [%5d] impossible to add new elements to the hash table of local entities\n", simclock, id);
				fflush(stdout);
				exit(-1);
			}
		}
	} else {
		// The model is unable to add the new SE in the global hash table
		fprintf(stdout, "%12.2f node: FATAL ERROR, [%5d] impossible to add new elements to the global hash table\n", simclock, id);
		fflush(stdout);
		exit(-1);
	}
}

/*
	Manages the "migration notification" of local SEs (i.e. allocated in this LP)
*/
static void notify_migration_event_handler (int id, int to) {

	hash_node_t *node;
	
	// The GAIA framework has decided that a local SE has to be migrated,
	//	the migration can NOT be executed immediately because the SE
	//	could be the destination of some "in flight" messages
	if ( ( node = hash_lookup ( table, id ) ) )  {
		/* Now it is updated the list of SEs that are enabled to migrate (flagged) */
		list_add ( mlist, node );

		node->data->lp			= to;
		node->data->s_state.changed	= YES;

		// Call the appropriate user event handler
		user_notify_migration_event_handler ();
	}

	// Just before the end of the current timestep, the migration list will be emptied
	//	and the pending migrations will be executed
}


/*
	Manages the "migration notification" of external SEs
	(that is, NOT allocated in the local LP)
*/
static void notify_ext_migration_event_handler (int id, int to) {

	hash_node_t *node;

	
	// A migration that does not directly involve the local LP is going to happen in
	//	the simulation. In some special cases the local LP has to take care of this simulation
	if ( ( node = hash_lookup ( table, id ) ) )  {
		node->data->lp			= to;		// Destination LP of the migration
		node->data->s_state.changed	= YES;

		// Call the appropriate user event handler
		user_notify_ext_migration_event_handler ();
	}
}


/*
	Migration-event manager (the real migration handler)
	This handler is executed when a migration message is received and therefore a new SE has to be accomodated in the local LP
*/
static void	migration_event_handler (int id, Msg *msg) {

	hash_node_t *node;

	if ( ( node = hash_lookup ( table, id ) ) ) {
		// Inserting the new SE in the local table
		hash_insert(LSE, stable, node->data, node->data->key, LPID);

		// Call the appropriate user event handler
		user_migration_event_handler(node, id, msg);
	}
}
/*---------------------------------------------------------------------------*/


/* ************************************************************************ */
/* 			   	    U T I L S				    */
/* ************************************************************************ */

/*
	Loading the configuration file of the simulator
*/
static void LoadINI(char *ini_name) {

	int	ret;	
	char	data[64];
	

	ret = INI_Load(ini_name);
	ASSERT( ret == INI_OK, ("Error loading ini file \"%s\"", ini_name) );
	
	/* SIMA */
        ret = INI_Read( "SIMA", "HOST", data);
	if (ret == INI_OK && strlen(data))
		strcpy(SIMA_HOST, data);

        ret = INI_Read( "SIMA", "PORT", data);
	if (ret == INI_OK && strlen(data))
		SIMA_PORT = atoi(data);

	INI_Free(); 
}
/*---------------------------------------------------------------------------*/


/* ************************************************************************ */
/* 			   	    M A I N				    */
/* ************************************************************************ */

int main(int argc, char* argv[]) {
	char 	msg_type, 		// Type of message
		*data,			// Buffer for incoming messages, dynamic allocation
		*rnd_file="Rand.seed";	// File containing seeds for the random numbers generator

	int	count, 			// Number of SEs to simulate in the local LP
		start,			// First identifier (ID) to be used to tag the locally managed SEs 
		max_data;		// Maximum size of incoming messages

	int	from,			// ID of the message sender 
		to;			// ID of the message receiver

	double  Ts;			// Current timestep
	Msg	*msg;			// Generic message

	struct hash_node_t		*tmp_node;			// Tmp variable, a node in the hash table
	char				*tmp_filename;			// File descriptors for simulation traces

	// Time measurement
	struct timeval 	t1,t2;		

	// Local PID
	local_pid = getpid();

	// Loading the input parameters from the configuration file
	LoadINI( "mig-agents.ini" );

	// Returns the standard host name for the execution host
	gethostname(LP_HOST, 64);

	// Command-line input parameters
	NLP		= atoi(argv[1]);	// Number of LPs
	NSIMULATE	= atoi(argv[2]);	// Number of SEs to simulate
	TESTNAME	= argv[3];		// Output directory for simulation traces

	/*
		Set-up of the GAIA framework

		Parameters:	
		1. (SIMULATE*NLP) 	Total number of simulated entities
		2. (NLP)	  	Number of LPs in the simulation
		3. (rnd_file)     	Seeds file for the random numbers generator 
		4. (NULL)         	LP canonical name
		5. (SIMA_HOST)	 	Hostname where the SImulation MAnager is running
		6. (SIMA_PORT)	  	SIMA TCP port number
	*/
	LPID = GAIA_Initialize ( NSIMULATE * NLP, NLP, rnd_file, NULL, SIMA_HOST, SIMA_PORT );
	
	// Returns the length of the timestep
	//	this value is defined in the "CHANNELS.TXT" configuration file
	//	given that GAIA is based on the time-stepped synchronization algorithm it retuns the size of a step
	step = GAIA_GetStep();

	// Due to synchronization constraints The FLIGHT_TIME has to be bigger than the timestep size
	if ( FLIGHT_TIME < step) {

		fprintf(stdout, "FATAL ERROR, the FLIGHT_TIME (%8.2f) is less than the timestep size (%8.2f)\n", FLIGHT_TIME, step);
		fflush(stdout);
		exit(-1);
	}

	// First identifier (ID) of SEs allocated in the local LP
	start	= NSIMULATE * LPID;

	// Number of SEs to allocate in the local LP
	count	= NSIMULATE;

	//  Used to set the ID of the first simulated entity (SE) in the local LP
	GAIA_SetFstID ( start );

	// User level handler to get some configuration parameters from the runtime environment (e.g. the GAIA parameters and many others)
	user_environment_handler();

	// Initialization of the random numbers generator
	RND_Init (S, rnd_file, LPID);
	
	// Data structures initialization (hash tables and migration list)
	hash_init ( table,  NSIMULATE * NLP );		// Global hashtable: all the SEs
	hash_init ( stable, NSIMULATE );		// Local hastable: local SEs
	list_init (mlist);				// Migration list (pending migrations in the local LP)

	// Starting the execution timer
	TIMER_NOW(t1);

	fprintf(stdout, "#LP [%d] HOSTNAME [%s]\n", LPID, LP_HOST);
	fprintf(stdout, "#                      LP[%d] STARTED\n#\n", LPID);

	fprintf(stdout, "#          Generating Simulated Entities from %d To %d ... ", ( LPID * NSIMULATE ), ( ( LPID * NSIMULATE ) + NSIMULATE ) - 1 );
	fflush(stdout);

	// Generate all the SEs managed in this LP
	Generate(count);
	fprintf(stdout, " OK\n#\n");

	fprintf(stdout, "# Data format:\n");
	fprintf(stdout, "#\tcolumn 1:	elapsed time (seconds)\n");
	fprintf(stdout, "#\tcolumn 2:	timestep\n");
	fprintf(stdout, "#\tcolumn 3:	number of entities in this LP\n");
	fprintf(stdout, "#\tcolumn 4:	number of migrating entities (from this LP)\n");

	// It is the LP that manages statistics
	if ( LPID == LP_STAT ) {		// Verbose output	
	
		fprintf(stdout, "#\tcolumn 5:	local communication ratio (percentage)\n");
		fprintf(stdout, "#\tcolumn 6:	remote communication ratio (percentage)\n");
		fprintf(stdout, "#\tcolumn 7:	total number of migrations in this timestep\n");
	}

	fprintf(stdout, "#\n");
	
	// Dynamically allocating some space to receive messages
	data = malloc(BUFFER_SIZE);
	ASSERT ((data != NULL), ("simulation main: malloc error, receiving buffer NOT allocated!"));
	
	// Before starting the real simulation tasks, the model level can initialize some
	//	data structures and set parameters
	user_bootstrap_handler();

	/* Main simulation loop, receives messages and calls the handler associated with them */
	while ( ! end_reached ) {
		// Max size of the next message. 
		// 	after the receive the variable will contain the real size of the message
		max_data = BUFFER_SIZE;

		// Looking for a new incoming message
		msg_type = GAIA_Receive( &from, &to,  &Ts, (void *) data, &max_data );
		msg 	 = (Msg *)data;

		// A message has been received, process it (calling appropriate handler)
		// 	message handlers
		switch ( msg_type ) {
			
			// The migration of a locally managed SE has to be done,
			//	calling the appropriate handler to insert the SE identifier in the list of pending migrations
			case NOTIF_MIGR:
				notify_migration_event_handler ( from, to );
			break;

			// A migration has been executed in the simulation but the local
			//	LP is not directly involved in the migration execution
			case NOTIF_MIGR_EXT:
				notify_ext_migration_event_handler ( from, to );
			break;

			// Registration of a new SE that is manager by another LP
			case REGISTER:
				register_event_handler ( from, to );
			break;

			// The local LP is the receiver of a migration and therefore a new
			//	SE has to be managed in this LP. The handler is responsible
			//	to allocate the necessary space in the LP data structures 
			//	and in the following to copy the SE state that is contained 
			//	in the migration message
			case EXEC_MIGR:
				migration_event_handler ( from, msg );
			break;

			// End Of Step:
			//	the current simulation step is finished, some pending operations
			//	have to be performed
			case EOS:
				// Stopping the execution timer 
				//	(to record the execution time of each timestep)
				TIMER_NOW ( t2 );
 
				/*  Actions to be done at the end of each simulated timestep  */
				if ( simclock < env_end_clock ) {	// The simulation is not finished

					// Simulating the interactions among SEs
					//
					// in the last (env_end_clock - FLIGHT_TIME) timesteps no pings will be sent
					// because we wanna check if all sent pings are correctly received
					if ( simclock < ( env_end_clock - FLIGHT_TIME ) ) { 

					    if (simclock < EXECUTION_STEP || (int)simclock % 10 != 0 ){
					    	Generate_Computation_and_Interactions( NSIMULATE * NLP );
				            } else {  //synchronize before executing the steps
				            	char tempFileName [15]; 
						snprintf(tempFileName, sizeof(tempFileName), "%s%d%s", "step", (int) (simclock), ".txt");						
						FILE *file = fopen(tempFileName, "r");
					        while (file == NULL ) {
						    file = fopen(tempFileName, "r");	
						    usleep (100000); //wait for 0.1s	
					        } 
					        Generate_Computation_and_Interactions( NSIMULATE * NLP );
					        fclose(file);
				            }
					} 
					
					simclock = GAIA_TimeAdvance();
				}
				else {
					/* End of simulation */
					TIMER_NOW(t2);
					/*
					fprintf(stdout, "\n\n");
					fprintf(stdout, "### Termination condition reached\n");
					fprintf(stdout, "### Clock           %12.2f\n", simclock);
					fflush(stdout);	
					*/
					end_reached = 1;
				}
			break;

			// Simulated model events (user level events)
			case UNSET:
				// First some checks for validation
				tmp_node = validation_model_events( from, to, msg );

				// The appropriate handler is defined at model level		
				user_model_events_handler( to, from, msg, tmp_node );
			break;

			default:
				fprintf(stdout, "FATAL ERROR, received an unknown event type: %d\n", msg_type);
				fflush(stdout);
				exit(-1);
		}
	}

	// Finalize the GAIA framework
	GAIA_Finalize();

	// Before shutting down, the model layer is able to deallocate some data structures
	user_shutdown_handler();
	
	// Freeing of the receiving buffer
	free(data);

	// Creating the "finished file" that is used by some scripts
	tmp_filename = malloc(256);
	snprintf(tmp_filename, 256, "%d.finished", LPID);
	finished_fp = fopen(tmp_filename, "w");
	fclose(finished_fp);

	// That's all folks.
	return 0;
}