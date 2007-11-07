#ifndef __ECL_RFT_NODE_H__
#define __ECL_RFT_NODE_H__


typedef struct ecl_rft_node_struct ecl_rft_node_type;

void                ecl_rft_node_fprintf_rft_obs(const ecl_rft_node_type * , const char * , const char * , double );
ecl_rft_node_type * ecl_rft_node_alloc(const ecl_block_type * );
const char        * ecl_rft_node_well_name_ref(const ecl_rft_node_type * );
void                ecl_rft_node_free(ecl_rft_node_type * );
void                ecl_rft_node_free__(void * );
void                ecl_rft_node_block(const ecl_rft_node_type *  , int , const double * , int * , int *  , int *);
const int 	  * ecl_rft_node_get_i (const ecl_rft_node_type * );
const int 	  * ecl_rft_node_get_j (const ecl_rft_node_type * );
const int 	  * ecl_rft_node_get_k (const ecl_rft_node_type * );
time_t    	    ecl_rft_node_get_recording_time(const ecl_rft_node_type * );
int                 ecl_rft_node_get_size(const ecl_rft_node_type * );

#endif

