/* Receive messages from node <SMECY_DOMAIN, PRODUCER_NODE, SEND_PORT> into
   <SMECY_DOMAIN, CONSUMER_NODE, RECEIVE_PORT>
*/

void consumer() {
#ifdef SMECY_VERBOSE
    fputs("Entering the consumer\n", stderr);
#endif
  mcapi_param_t parameters;
  mcapi_info_t info;
  mcapi_endpoint_t data_transmit_endpoint;
  mcapi_endpoint_t data_receive_endpoint;

  mcapi_status_t status;
  mcapi_initialize(SMECY_DOMAIN, CONSUMER_NODE,
		   &parameters, &info, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t receive = mcapi_endpoint_create(RECEIVE_PORT, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < 100; i++) {
    char buffer[100];
    size_t received_size;
    mcapi_msg_recv(receive, buffer, sizeof(buffer), &received_size, &status);
    MCAPI_CHECK_STATUS(status);
    /* Avoid later over-reading: */
    buffer[sizeof(buffer) - 1] = '\0';
    printf("Received %zd characters: %s\n", received_size, buffer);
  }

  mcapi_finalize(&status);
  MCAPI_CHECK_STATUS(status);
#ifdef SMECY_VERBOSE
    fputs("Exiting the consumer\n", stderr);
#endif
}
