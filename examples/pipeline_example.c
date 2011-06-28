#include <stdlib.h>
#include <stdio.h>

#define buffer_length 128
typedef int data_buff[buffer_length];

/* Produce a random buffer

   @param[out] data_buffer is an array initialized with random numbers
*/
void Produce(data_buff data_buffer) {
  for(int i = 0; i < buffer_length; i++)
    /* Note that rand() is not thread-safe but it is OK for this
       example */
    data_buffer[i] = rand();
}

/* Compute the average value of an array and display it

   @param[in] data_buffer is the array to analyze
*/
void Consume(data_buff data_buffer) {
  double average = 0;
  for(int i = 0; i < buffer_length; i++)
    /* Note that rand() is not thread-safe but it is OK for this
       example */
    average += data_buffer[i];
  // Normalize:
  average /= RAND_MAX;
  average /= buffer_length;
  printf("Average = %f\n", average);
}


int main() {
  data_buff data_buffer;
  /**/
#pragma smecy stream_loop
  while(1) {
#pragma smecy stream_node(1)
    Produce(data_buffer);
#pragma smecy stream_node(2)
    Consume(data_buffer);
  }
  return 0;
}
