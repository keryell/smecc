#ifndef P4A_MACROS_H
#define P4A_MACROS_H

#include "smecy.h"

#define P4A_MAX_STREAMING_LOOPS 64
#define P4A_MAX_STREAMING_NODES 32

static int p4a_next_stream = 0;
DbLink p4a_streaming_buffers[P4A_MAX_STREAMING_LOOPS][P4A_MAX_STREAMING_NODES];

//TODO for loop and nbNodes param
#define p4a_init_stream(stream, nbstreams)\
{\
	for (int p4a_iter_##stream=0; p4a_iter_##stream<(nbstreams); p4a_iter_##stream++)\
	{\
		p4a_streaming_buffers[p4a_next_stream][p4a_iter_##stream] = pth_CreateDbLink(sizeof(__buffer_type_##stream)); \
	}\
	p4a_next_stream++; \
}

#define p4a_launch_stream(stream, node)\
	pth_CreateProcess(((int (*)())__Node_##stream##_##node))
	
#define p4a_stream_get_init_buf(stream, node)\
    struct_buffer_out = ((__buffer_type_##stream *)(DbLinkGetInitBuf(p4a_streaming_buffers[stream][node])))
    
#define p4a_stream_put_data(stream, node)\
    struct_buffer_out = ((__buffer_type_##stream *)(DbLinkPutData(p4a_streaming_buffers[stream][node])))
      
#define p4a_stream_get_data(stream, node)\
	struct_buffer_in = ((__buffer_type_##stream *)(DbLinkPutData(p4a_streaming_buffers[stream][node])));

#define p4a_stream_copy_data(stream, node)\
	struct_buffer_out = struct_buffer_in;



#endif