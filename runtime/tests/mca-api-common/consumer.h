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

  /* First use communications with messages (connection-less mode) */

  mcapi_endpoint_t msg_receive = mcapi_endpoint_create(RECEIVE_MSG_PORT,
						       &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < N_MSG; i++) {
    /* Messages (ie for connection-less communications) have to be
       provided by the user in MCAPI: */
    char buffer[100];
    size_t received_size;
    mcapi_msg_recv(msg_receive, buffer, sizeof(buffer), &received_size, &status);
    MCAPI_CHECK_STATUS(status);
    /* Avoid later over-reading: */
    buffer[sizeof(buffer) - 1] = '\0';
    printf("Received %zd characters with a message: %s\n",
	   received_size, buffer);
  }
  // Delete this endpoint which is useless now:
  mcapi_endpoint_delete(msg_receive, &status);
  MCAPI_CHECK_STATUS(status);


  /* Then use communications with packets (connected mode) */

  mcapi_endpoint_t pkt_receive = mcapi_endpoint_create(RECEIVE_PKT_PORT,
						       &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_pktchan_send_hndl_t receive_gate;
  mcapi_request_t handle;
  // Let the sender do the connection and open for receive
  mcapi_pktchan_recv_open_i(&receive_gate, pkt_receive, &handle, &status);
  MCAPI_CHECK_STATUS(status);
  size_t size;
  // Wait for the completion of opening
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < N_MSG; i++) {
    char *message;
    size_t received_size;
    mcapi_pktchan_recv(receive_gate, (void **)&message,
		       &received_size, &status);
    MCAPI_CHECK_STATUS(status);
    printf("Received %zd characters with a packet: %*s\n",
	   received_size, received_size, message);
    // Give back the memory message to the library:
    mcapi_pktchan_release(message, &status);
    MCAPI_CHECK_STATUS(status);
  }

  // Now we can close the receive side of the channel
  mcapi_pktchan_recv_close_i(receive_gate, &handle, &status);
  MCAPI_CHECK_STATUS(status);
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  // Remove useless the receiving endpoint
  mcapi_endpoint_delete(pkt_receive, &status);
  MCAPI_CHECK_STATUS(status);

  /* Release the API use */
  mcapi_finalize(&status);
  MCAPI_CHECK_STATUS(status);
#ifdef SMECY_VERBOSE
    fputs("Exiting the consumer\n", stderr);
#endif
}
