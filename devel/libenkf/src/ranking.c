/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'misfit_ranking.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <util.h>
#include <hash.h>
#include <vector.h>
#include <double_vector.h>
#include <msg.h>
#include <buffer.h>

#include <enkf_obs.h>
#include <enkf_fs.h>
#include <enkf_util.h>
#include <misfit_ranking.h>

/**
   This struct contains the misfits & sort keys for one particular
   misfit_ranking. I.e. all the RFT measurements.
*/



#define MISFIT_RANKING_TYPE_ID      671108

struct misfit_ranking_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type        * ensemble;             /* An ensemble of hash instances. Each hash instance is populated like this: hash_insert_double(hash , "WGOR" , 1.09); */
  double_vector_type * total;                /* An enemble of total misfit values (for this misfit_ranking). */
  int                * sort_permutation;     /* This is how the ens members should be permuted to be sorted under this misfit_ranking.                                     */
};

static UTIL_SAFE_CAST_FUNCTION( misfit_ranking , MISFIT_RANKING_TYPE_ID )

static void misfit_ranking_display_misfit__( const misfit_ranking_type * misfit_ranking , FILE * stream) {
  fprintf(stream,"\n\n");
  fprintf(stream,"  #    Realization    Normalized misfit    Total misfit\n");
  fprintf(stream,"-------------------------------------------------------\n");
  for (i = 0; i < ens_size; i++) {
    int    iens         = permutations[i];
    double total_misfit = double_vector_iget( misfit_ranking->total , iens );
    double normalized_misfit = sqrt(total_misfit / num_obs_total);
    summed_up = summed_up+total_misfit;
    fprintf(stream,"%3d    %3d                   %10.3f      %10.3f  \n",i,iens,normalized_misfit,total_misfit);
  }
  
  {
    double normalized_summed_up = sqrt(summed_up / (num_obs_total * ens_size));
    fprintf(stream,"        All                  %10.3f      %10.3f  \n",normalized_summed_up,summed_up);
  }
  fprintf(stream,"-------------------------------------------------------\n");
}




void misfit_ranking_display( const misfit_ranking_type * misfit_ranking ) {
  FILE * stream = stdout;
  const int ens_size                  = double_vector_size( misfit_ranking->total );
  const int * permutations            = misfit_ranking->sort_permutation;
  hash_type * obs_hash = NULL;
  {
    // The ensemble vector can contain invalid nodes with NULL.
    int index = 0;
    while ((obs_hash == NULL) && (index < vector_get_size( misfit_ranking->ensemble))) {
      obs_hash = vector_iget( misfit_ranking->ensemble , index );
      index++;
    }
    if (obs_hash == NULL) {
      fprintf(stderr,"Sorry: no valid results loaded for this misfit_ranking - returning\n");
      return;
    }
  }
  
  {
    int i;
    double summed_up = 0.0;
    stringlist_type * obs_keys          = hash_alloc_stringlist( obs_hash );
    int num_obs                         = stringlist_get_size( obs_keys );
    int num_obs_total                   = num_obs * ens_size;  // SHould not count failed/missing members ...
    misfit_ranking_display_misfit__( misfit_ranking , stream );
  }
  
}



void misfit_ranking_fprintf( const misfit_ranking_type * misfit_ranking , const char * filename) {
  FILE * stream                       = util_mkdir_fopen( filename , "w");
  const int ens_size                  = vector_get_size( misfit_ranking->sort_permutation );
  const int * permutations            = misfit_ranking->sort_permutation;
  double summed_up = 0.0;
  {
    // All this whitespace is finely tuned and highly significant .... 
    const char * key_fmt       = " %18s ";                                
    const char * value_fmt     = " %10.3f %8.3f";
    const char * start_fmt     = " %2d       %3d     %7.3f %8.3f";  

    hash_type * obs_hash       = vector_iget( misfit_ranking->ensemble , 0);
    stringlist_type * obs_keys = hash_alloc_stringlist( obs_hash );
    int num_obs                = stringlist_get_size( obs_keys );
    int iobs;
    int num_obs_total = num_obs * ens_size;

    stringlist_sort( obs_keys , enkf_util_compare_keys__ );
    fprintf(stream , "                       Overall  ");
    for (iobs =0; iobs < num_obs; iobs++) 
      fprintf(stream , key_fmt , stringlist_iget( obs_keys , iobs ));

    fprintf(stream , "\n");
    fprintf(stream , "  #    Realization  Norm    Total");
    for (iobs =0; iobs < num_obs; iobs++) 
      fprintf(stream , "       Norm    Total");
    
    fprintf(stream , "\n");
    for (int i = 0; i < ens_size; i++) {
      int iens = permutations[i];
      hash_type * obs_hash = vector_iget( misfit_ranking->ensemble , iens );
      double total_value   = double_vector_iget( misfit_ranking->total , iens );
      double normalized_misfit = sqrt(total_value / num_obs_total);
      summed_up = summed_up+total_value;
      fprintf(stream , start_fmt , i , iens , normalized_misfit , total_value);
      for (iobs =0; iobs < num_obs; iobs++){
        double single_value = hash_get_double( obs_hash , stringlist_iget( obs_keys , iobs ));
        double single_value_normalized = sqrt(single_value / (num_obs_total));
        fprintf(stream , value_fmt , single_value_normalized , single_value);
      }
      fprintf(stream , "\n");
    }
    double summed_up_normalized = sqrt(summed_up / (num_obs_total * ens_size));
    fprintf(stream , "           All    %7.3f %8.3f" , summed_up_normalized , summed_up);
    for (iobs = 0; iobs < num_obs; iobs++){
      double single_value_summed_up = 0.0;      
      for (int i = 0; i < ens_size; i++) {  
        single_value_summed_up = single_value_summed_up + hash_get_double( obs_hash , stringlist_iget( obs_keys , iobs ));
      }
      double single_value_summed_up_normalized=sqrt(single_value_summed_up / (num_obs_total * ens_size));
      fprintf(stream , value_fmt , single_value_summed_up_normalized , single_value_summed_up);
    }
    fprintf(stream , "\n");
  }
  fclose( stream );
}


misfit_ranking_type * misfit_ranking_alloc( ) {
  misfit_ranking_type * misfit_ranking = util_malloc( sizeof * misfit_ranking );
  UTIL_TYPE_ID_INIT( misfit_ranking , MISFIT_RANKING_TYPE_ID );
  misfit_ranking->sort_permutation = NULL;
  misfit_ranking->ensemble = vector_alloc_new();
  misfit_ranking->total    = double_vector_alloc( 0 , INVALID_MISFIT );
  return misfit_ranking;
}


void misfit_ranking_free( misfit_ranking_type * misfit_ranking ) {
  vector_free( misfit_ranking->ensemble );
  double_vector_free( misfit_ranking->total );
  util_safe_free( misfit_ranking->sort_permutation );
  free( misfit_ranking );
}



void misfit_ranking_free__( void * arg ) {
  misfit_ranking_type * misfit_ranking = misfit_ranking_safe_cast( arg );
  misfit_ranking_free( misfit_ranking );
}



void misfit_ranking_iset( misfit_ranking_type * misfit_ranking , int iens , hash_type * obs_hash , double total_misfit) {
  if (iens > vector_get_size(misfit_ranking->ensemble))
    vector_grow_NULL( misfit_ranking->ensemble , iens );
  
  vector_iset_owned_ref( misfit_ranking->ensemble , iens , obs_hash , hash_free__ );
  double_vector_iset( misfit_ranking->total , iens , total_misfit );
}


void misfit_ranking_iset_invalid( misfit_ranking_type * misfit_ranking , int iens ) {
  misfit_ranking_iset( misfit_ranking , iens , NULL , INVALID_MISFIT );
}

void misfit_ranking_init_sort( misfit_ranking_type * misfit_ranking ) {
  misfit_ranking->sort_permutation = double_vector_alloc_sort_perm( misfit_ranking->total );
}

const int * misfit_ranking_get_permutation( const misfit_ranking_type * misfit_ranking ) {
  return misfit_ranking->sort_permutation;
}
