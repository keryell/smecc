#include "p4a_macros.h" 

void stuff(int thing)
{
}

void stuff2(int thing,int other_thing)
{
}

typedef struct
{
  int b;
  int c;
} __buffer_type_0
;

void __Node_0_0()
{
    __buffer_type_0* struct_buffer;
    p4a_stream_get_init_buf(0);
    while(1){
      stuff(struct_buffer->b);
      p4a_stream_put_data(0);
    }
}

void __Node_0_1()
{
    __buffer_type_0 *struct_buffer;
    while(1){
      p4a_stream_get_data(0);
      stuff2(struct_buffer->b,struct_buffer->c);
    }
}

int main()
{
  unsigned int tab[10UL][100UL];
  int b = (sizeof(int ));
  int c = 3;
  p4a_init_stream(0);
  p4a_launch_stream(0,0);
  p4a_launch_stream(0,1);
  pause();
  return 0;
}
