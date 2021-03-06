/* Send messages from node <PRODUCER_DOMAIN, PRODUCER_NODE, SEND_PORT> to
   <CONSUMER_DOMAIN, CONSUMER_NODE, RECEIVE_PORT>
*/
void producer(mcapi_param_t *parameters) {
  mcapi_info_t info;
  mcapi_endpoint_t data_transmit_endpoint;
  mcapi_endpoint_t data_receive_endpoint;

  mcapi_status_t status;

#ifdef SMECY_VERBOSE
    fputs("Entering the producer\n", stderr);
#endif

  // init node attributes. Not clear in which MCAPI version it is needed...
  /* It looks like in the Linux MCAPI implementation reference from MCA,
     even the 2.015 version looks like a V1 interface... */
#if (MCAPI_VERSION >= 2000)
  mcapi_node_attributes_t node_attributes;
  mcapi_node_init_attributes(&node_attributes, &status);
  MCAPI_CHECK_STATUS(status);
  /* 6 arguments in V.2 */
  mcapi_initialize(PRODUCER_DOMAIN, PRODUCER_NODE, &node_attributes,
  		   parameters, &info, &status);
#else
  /* 5 arguments in V.1 */
  mcapi_initialize(PRODUCER_DOMAIN, PRODUCER_NODE,
		   parameters, &info, &status);
  MCAPI_CHECK_STATUS(status);
#endif

  /* First use communications with messages (connection-less mode) */

  mcapi_endpoint_t msg_send = mcapi_endpoint_create(SEND_MSG_PORT, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t msg_receive = mcapi_endpoint_get(CONSUMER_DOMAIN,
						    CONSUMER_NODE,
						    RECEIVE_MSG_PORT,
						    MCA_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < N_MSG; i++) {
    /* Messages (ie for connection-less communications) have to be
       provided by the user in MCAPI: */
#ifndef MCAPI_STHORM
    char message[100];
    snprintf(message, sizeof(message), "Message: Salut %d", i);
#else
    /* There is no snprintf() on STHORM */
    char message[] = "Message: Salut @";
    message[sizeof(message) - 2] = '0' + i;
#endif
    /* There is no control flow, so we could reach a memory limit here
       (MCAPI_ERR_MEM_LIMIT).  Not to be done this way in real life, but
       here it is just for testing. We hope N_MSG is low enough to avoid
       any problemm here */
    mcapi_msg_send(msg_send, msg_receive, message, sizeof(message),
		   MCAPI_MAX_PRORITY, &status);
    MCAPI_CHECK_STATUS(status);
  }
  // Delete the endpoint we created which is useless now
  mcapi_endpoint_delete(msg_send, &status);
  MCAPI_CHECK_STATUS(status);

  MCAPI_TRACE_C("Communications with packets (connected mode)");
  /* Then use communications with packets (connection mode) */

  mcapi_endpoint_t pkt_send = mcapi_endpoint_create(SEND_PKT_PORT, &status);
  MCAPI_CHECK_STATUS(status);

#ifdef MCAPI_STHORM
  /* On STHORM, it looks like we need to precise this attribute to have a
     connection with the host working */
  mcapi_endp_attr_memory_type_t memtype = MCAPI_ENDP_ATTR_REMOTE_MEMORY;
  mcapi_endpoint_set_attribute(pkt_send,
                               MCAPI_ENDP_ATTR_MEMORY_TYPE,
                               &memtype,
                               sizeof(mcapi_endp_attr_memory_type_t),
                               &status);
  MCAPI_CHECK_STATUS(status);
#endif

  mcapi_endpoint_t pkt_receive = mcapi_endpoint_get(CONSUMER_DOMAIN,
						    CONSUMER_NODE,
						    RECEIVE_PKT_PORT,
						    MCA_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_request_t handle;
  // In an example from STHORM MCAPI they use a loop like this one
  // but it seems working without it here
  //// Connect & wait for the connection
  //do {
    // Start a connection request...
    mcapi_pktchan_connect_i(pkt_send, pkt_receive, &handle, &status);
  //} while (status==MCAPI_ERR_ATTR_INCOMPATIBLE);
  MCAPI_CHECK_STATUS(status);

  // ...and wait for its completion
  size_t size;
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_pktchan_send_hndl_t send_gate;
  // Open this side of the channel for sending
  mcapi_pktchan_send_open_i(&send_gate, pkt_send, &handle, &status);
  MCAPI_CHECK_STATUS(status);
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < N_MSG; i++) {
#ifndef MCAPI_STHORM
    char message[100];
    snprintf(message, sizeof(message), "Packet: Mont a ra mat %d", i);
#else
    /* There is no snprintf() on STHORM */
    char message[] = "Packet: Mont a ra mat @";
    message[sizeof(message) - 2] = '0' + i;
#endif
    // Send a packet
    mcapi_pktchan_send(send_gate, message, sizeof(message), &status);
    MCAPI_CHECK_STATUS(status);
  }

  // Close the send side of the channel
  MCAPI_TRACE_C("Closing the producer channel");
  mcapi_pktchan_send_close_i(send_gate, &handle, &status);
  MCAPI_CHECK_STATUS(status);
  MCAPI_TRACE_C("Waiting for the closing");
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  // Remove useless the sending endpoint
  MCAPI_TRACE_C("Delete the end point");
  mcapi_endpoint_delete(pkt_send, &status);
  MCAPI_CHECK_STATUS(status);

  MCAPI_TRACE_C("Finalizing...");
  /* Release the API use */
  mcapi_finalize(&status);
  MCAPI_CHECK_STATUS(status);

#ifdef SMECY_VERBOSE
    fputs("Exiting the producer\n", stderr);
#endif
}
