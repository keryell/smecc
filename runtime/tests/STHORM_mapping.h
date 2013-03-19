/* Give the MCAPI layout on STHORM */
enum {
  /* Put The producer on the PE 3 of cluster 1 in the hardware accelerator
     fabric */
  PRODUCER_DOMAIN = 1,
  PRODUCER_NODE = 3,
  /* Put the consumer on the host node */
  CONSUMER_DOMAIN = 5,
  CONSUMER_NODE = 0,
  /* There are strong limits on the following values in the
     STHORM implementation */
  SEND_MSG_PORT = 2,
  RECEIVE_MSG_PORT = 3,
  SEND_PKT_PORT = 10,
  RECEIVE_PKT_PORT = 11,
  N_MSG = 5
};
