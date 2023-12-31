/*	##############################################################################################
	Advanced RTI System, ARTÌS			http://pads.cs.unibo.it
	Large Unstructured NEtwork Simulator (LUNES)

	Description:
		-	Utility functions, in particular data structures management
			(e.g. hash table, list)

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
#include "utils.h"

extern double			simclock;						/* Time management, simulated time */


/* ************************************************************************* */
/* 	         D A T A    S T R U C T U R E S    M A N A G E M E N T       */
/* ************************************************************************* */

int hash(hash_t *tptr, int x) {

	return  (x % tptr->size);
}


/*
	Hash table initialization
*/
void hash_init(hash_t *tptr, int size) {

   	tptr->size	= size;
	tptr->count	= 0;
   	tptr->bucket	= (hash_node_t **) calloc(tptr->size, sizeof(hash_node_t *));

   	return;
}


/*
	Lookup of a simulated entity (hash table)
*/
hash_node_t *hash_lookup(hash_t *tptr, int key) {

   	int 		h;
	hash_node_t 	*node;


   	h = hash(tptr, key);
   	for (node=tptr->bucket[h]; node!=NULL; node=node->next) {
     		if (node->data->key == key)
      			break;
   	}

   	return(node);
}


/*
	Insertion of a new simulated entity (hash table)
*/
hash_node_t * hash_insert(enum HASH_TYPE type, hash_t *tptr, struct hash_data_t *data, int key, int lp) {
	//fprintf(stdout, "%d\n", (int)simclock);
   	hash_node_t 	*node, *tmp;
   	int h;


	if ( (tmp=hash_lookup(tptr, key)) )
		return(tmp);
	
	h = hash(tptr, key);

	node = (struct hash_node_t *) malloc(sizeof(hash_node_t));
	ASSERT ((node != NULL), ("hash_insert: malloc error"));

	//	Inserting the SE in the global hashtable
	if(type == GSE) {
		node->data	 = (struct hash_data_t *) malloc(sizeof(hash_data_t));
		ASSERT ((node->data != NULL), ("hash_insert: malloc error"));

		node->data->key	   	= key;
		node->data->lp	   	= lp;
		//node->data->dataCenterID = -1;
		//node->data->subnetId = -1;

	}		//	Inserting the SE in the local hashtable
	else if(type == LSE) {
		node->data 		= data;
		node->data->lp	= lp;
		node->data->numBlocks = 0;
	}

   	node->next	= tptr->bucket[h];
   	tptr->bucket[h]	= node;
    tptr->count	+= 1;
		
   	return node;
}


/*
	Deletion of a node (hash table)
*/
int hash_delete(enum HASH_TYPE type, hash_t *tptr, int key) {

	hash_node_t *node, *last;
   	int h;


   	h = hash(tptr, key);
   	for (node=tptr->bucket[h]; node; node=node->next) {
     		if (node->data->key == key)
       			break;
   	}


   	if (node==NULL)
     		return -1;


   	if (node == tptr->bucket[h])
     		tptr->bucket[h] = node->next;
   	else {
     		for (last=tptr->bucket[h]; last && last->next; last=last->next) {
       			if (last->next == node)
         			break;
     		}
     		last->next = node->next;
   	}

	tptr->count	-= 1;
	
	if(type == GSE)
		free(node->data);

	free(node);

   	return(1);
}


/*---------------------------------------------------------------------------*/


/*
	List initialization
*/
void	list_init (se_list  *list) {

	list->head	= NULL;
	list->tail	= NULL;
	list->size 	= 0;
}


/*
	Insertion of a new node (list)
*/
void	list_add (se_list  *list, hash_node_t *node) {

	se_list_n 	*list_n;


   	list_n		  = (struct se_list_n *) malloc(sizeof(se_list_n));
	ASSERT ((list_n != NULL), ("list_add: malloc error"));

   	list_n->node	  = node;
   	list_n->next	  = NULL;


	if(list->head == NULL) {
		list->head = list_n;
	}

	
	if(list->tail != NULL) {
		list->tail->next = list_n;
	}

	list->tail 	 = list_n;
	list->size	+= 1;
}


/*
	Deletion of a node (list)
*/
struct hash_node_t *list_del (se_list  *list) {

	struct se_list_n  *head=NULL;
	struct hash_node_t *node=NULL;

	
	if(list->size > 0) {
		list->size     -= 1;

		head 		= list->head;
		node 		= list->head->node;
		list->head	= list->head->next;
		
		if( list->size <= 0 ) 
			list->tail = NULL;
			
		free(head);
	}
	

	return node;
}
/*---------------------------------------------------------------------------*/


