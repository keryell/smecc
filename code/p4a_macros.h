#ifndef P4A_MACROS_H
#define P4A_MACROS_H

#include <smecy.h>

#define P4A_MAX_STREAMING_BUFFERS 64
#define P4A_MAX_STREAMING_BUFFER_SIZE 1024

static p4a_next_stream = 0;
DbLink p4a_streaming_buffers[P4A_MAX_STREAMING_BUFFERS];

#define p4a_init_stream(stream)\
{\
	p4a_streaming_buffers[p4a_next_stream] = pth_CreateDbLink(sizeof(__buffer_type_##stream)); \
	p4a_next_stream++; \
}

#define p4a_launch_stream(stream, node)\
	pth_CreateProcess(((int (*)())__Node_##stream##_##node),__stream_buffer_##stream)
	
#define p4a_arg(arg)\
	struct_buffer->arg
	
#define p4a_def_begin_node(stream, node, condition, funccall)\
void __Node_##stream##_##node(DbLink data_buffer)\
{\
    __buffer_type_##stream *struct_buffer; \
    struct_buffer = ((__buffer_type_##stream *)(DbLinkGetInitBuf(data_buffer))); \
    while(condition){ \
      funcall; \
      struct_buffer = ((__buffer_type_##stream *)(DbLinkPutData(data_buffer))); \
    } \
}


#endif