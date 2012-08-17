/* Send messages from node <SMECY_DOMAIN, PRODUCER_NODE, SEND_PORT> to
   <SMECY_DOMAIN, CONSUMER_NODE, RECEIVE_PORT>
*/

void producer() {
  //mcapi_node_attributes_t node_attributes;
  mcapi_param_t parameters;
  mcapi_info_t info;
  mcapi_endpoint_t data_transmit_endpoint;
  mcapi_endpoint_t data_receive_endpoint;

  mcapi_status_t status;

#ifdef SMECY_VERBOSE
    fputs("Entering the producer\n", stderr);
#endif

  // init node attributes. Not clear in which MCAPI version it is needed...
  //mcapi_node_init_attributes(&node_attributes, &status);
  //MCAPI_CHECK_STATUS(status);
  //mcapi_initialize(SMECY_DOMAIN, PRODUCER_NODE, &node_attributes,
  //		   &parameters, &info, &status);
  mcapi_initialize(SMECY_DOMAIN, PRODUCER_NODE,
		   &parameters, &info, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t send = mcapi_endpoint_create(SEND_PORT, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t receive = mcapi_endpoint_get(SMECY_DOMAIN,
						CONSUMER_NODE,
						RECEIVE_PORT,
						MCA_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < 100; i++) {
    char message[100];
    snprintf(message, sizeof(message), "Salut %d", i);
    /* There is no control flow, so we could reach a memory limit here
       (MCAPI_ERR_MEM_LIMIT).  Not to be done this way in real life, but
       here it is just for testing. */
    mcapi_msg_send(send, receive, message, sizeof(message),
		   MCAPI_MAX_PRORITY, &status);
    MCAPI_CHECK_STATUS(status);
  }

  mcapi_finalize(&status);
  MCAPI_CHECK_STATUS(status);

#ifdef SMECY_VERBOSE
    fputs("Exiting the producer\n", stderr);
#endif
}
