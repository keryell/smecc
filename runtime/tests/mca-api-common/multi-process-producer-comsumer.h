/* Use another domain number from mono-process OpenMP version to be able to
   run as the same time as this multi-process version */
enum {
  PRODUCER_DOMAIN = 5,
  PRODUCER_NODE = 3,
  CONSUMER_DOMAIN = 0,
  CONSUMER_NODE = 7,
  SEND_MSG_PORT = 11,
  RECEIVE_MSG_PORT = 13,
  SEND_PKT_PORT = 41,
  RECEIVE_PKT_PORT = 43,
  N_MSG = 10
};
