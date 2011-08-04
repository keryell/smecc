#ifndef P4A_MACROS_H
#define P4A_MACROS_H

#include "smecy.h"

#define P4A_MAX_STREAMING_BUFFERS 64
#define P4A_MAX_STREAMING_BUFFER_SIZE 1024

static int p4a_next_stream = 0;
DbLink p4a_streaming_buffers[P4A_MAX_STREAMING_BUFFERS];

#define p4a_init_stream(stream)\
{\
	p4a_streaming_buffers[p4a_next_stream] = pth_CreateDbLink(sizeof(__buffer_type_##stream)); \
	p4a_next_stream++; \
}

#define p4a_launch_stream(stream, node)\
	pth_CreateProcess(((int (*)())__Node_##stream##_##node))
	
#define p4a_stream_get_init_buf(stream)\
    ((__buffer_type_##stream *)(DbLinkGetInitBuf(p4a_streaming_buffers[stream])))
    
#define p4a_stream_put_data(stream)\
      struct_buffer = ((__buffer_type_##stream *)(DbLinkPutData(p4a_streaming_buffers[stream])))
      
#define p4a_stream_get_data(stream)\
	struct_buffer = ((__buffer_type_##stream *)(DbLinkPutData(p4a_streaming_buffers[stream])));

#endif