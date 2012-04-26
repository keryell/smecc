#ifndef P4A_MACROS_H
#define P4A_MACROS_H

#include <unistd.h>
#include "smecy.h"

#define P4A_MAX_STREAMING_LOOPS 64
#define P4A_MAX_STREAMING_NODES 32

static int p4a_next_stream = 0;
DbLink p4a_streaming_buffers[P4A_MAX_STREAMING_LOOPS][P4A_MAX_STREAMING_NODES];

//TODO for loop and nbNodes param
#define P4A_init_stream(stream, nbstreams)\
{\
	for (int p4a_iter_##stream=0; p4a_iter_##stream<(nbstreams); p4a_iter_##stream++)\
	{\
		p4a_streaming_buffers[p4a_next_stream][p4a_iter_##stream] = pth_CreateDbLink(sizeof(struct p4a_buffer_type_##stream)); \
	}\
	p4a_next_stream++; \
}

#define P4A_launch_stream(stream, node)\
	pth_CreateProcess(((int (*)())p4a_node_##stream##_##node))

#define P4A_stream_get_init_buf(stream, node)\
    p4a_struct_buffer_out = ((struct p4a_buffer_type_##stream *)(DbLinkGetInitBuf(p4a_streaming_buffers[stream][node])))

#define P4A_stream_put_data(stream, node)\
    p4a_struct_buffer_out = ((struct p4a_buffer_type_##stream *)(DbLinkPutData(p4a_streaming_buffers[stream][node])))

#define P4A_stream_get_data(stream, node)\
	p4a_struct_buffer_in = ((struct p4a_buffer_type_##stream *)(DbLinkPutData(p4a_streaming_buffers[stream][node])));

#define P4A_stream_copy_data(stream, node)\
	p4a_struct_buffer_out = p4a_struct_buffer_in;

/* Wait to the end of the application with Unix system-call: */
#define P4A_wait_for_the_end pause
#endif
