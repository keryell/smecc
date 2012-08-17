/* Use another domain number from mono-process OpenMP version to be able to
   run as the same time as this multi-process version */
enum {
  SMECY_DOMAIN = 23,
  PRODUCER_NODE = 3,
  CONSUMER_NODE = 7,
  SEND_PORT = 11,
  RECEIVE_PORT = 13
};
