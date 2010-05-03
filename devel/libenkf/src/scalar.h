#ifndef __SCALAR_H__
#define __SCALAR_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <enkf_util.h>
#include <enkf_serialize.h>
#include <stdio.h>
#include <buffer.h>

typedef struct scalar_struct scalar_type;

void             scalar_truncate(scalar_type * );
void             scalar_get_data(const scalar_type * , double * );
void             scalar_get_output_data(const scalar_type * , double * );
void             scalar_set_data(scalar_type * , const double * );
void             scalar_resize(scalar_type * scalar , int size);
scalar_type      * scalar_alloc( int size );
void             scalar_free(scalar_type *);
void             scalar_sample(scalar_type *);
void             scalar_truncate(scalar_type *);
void             scalar_stream_fwrite(const scalar_type * scalar , FILE * , bool);
void             scalar_stream_fread(scalar_type * scalar , FILE * );
void             scalar_buffer_fload(scalar_type * scalar , buffer_type * buffer);
void             scalar_buffer_fsave(const scalar_type * scalar , buffer_type * buffer , bool internal_state);
void             scalar_realloc_data(scalar_type * scalar);
void             scalar_clear(scalar_type * scalar); 
const double   * scalar_get_output_ref(const scalar_type * );
double         * scalar_get_data_ref  (const scalar_type * );
double           scalar_iget_double(scalar_type * , bool , int );
void             scalar_memcpy(scalar_type * , const scalar_type * );
void             scalar_free_data(scalar_type *);
void 		 scalar_matrix_deserialize(scalar_type * scalar , const active_list_type * active_list , const matrix_type * A , int row_offset , int column);
void 		 scalar_matrix_serialize(const scalar_type *scalar ,  const active_list_type * active_list , matrix_type * A , int row_offset , int column);
void             scalar_set_inflation(scalar_type * inflation , const scalar_type * std , const scalar_type * min_std);
void             scalar_scale(scalar_type * scalar, double factor);
void             scalar_iset(scalar_type * scalar , int index , double value);



MATH_OPS_HEADER(scalar);

#ifdef __cplusplus
}
#endif
#endif
