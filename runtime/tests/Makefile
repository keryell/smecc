# Makefile to test some runtime functions

ifndef TARGET
  $(error You have to launch this Makefile with TARGET=MCA or TARGET=STHORM to select the MCAPI implementation. For the STHORM target you can add FABRIC_TYPE=posix or iss, and HOST_TYPE= posix or arm. posix is the default for both)
endif

ifeq ($(TARGET),MCA)
  LOCAL_BIN=producer-consumer mca-api-openmp mca-api-producer mca-api-consumer
  SMECY_INC=..

  # export MCA_INCLUDE=/usr/local/mca-api/include

  MCA_INCLUDE:=`mcapi-config --cflags`
  MCAPI_LINK:=`mcapi-config --libs`
  MRAPI_LINK:=`mrapi-config --libs`
  LDLIBS+=$(MCAPI_LINK) $(MRAPI_LINK)

  compile: $(LOCAL_BIN)

  include ../use.mk
endif

ifeq ($(TARGET),STHORM)
  # Special definitions for STHORM target platform:
  HOST_SRC = STHORM_host.c
  FABRIC_SRC = STHORM_fabric.c
  HOST_DEP = mca-api-common/smecy-mca-common.h STHORM_mapping.h mca-api-common/consumer.h
  FABRIC_DEP = mca-api-common/smecy-mca-common.h STHORM_mapping.h mca-api-common/producer.h
  HOST_EXE = STHORM_host
  # The name of the library that will contain the accelerator part:
  FABRIC_LIB = fabric

  # -U MCAPI_TRACE_ON
  CFLAGS += -DMCAPI_STHORM -g -std=c99
  FABRIC_CFLAGS += -DMCAPI_STHORM -g -std=c99

  ifndef P12MCAPI
    $(error You have to set the P12MCAPI environment variable to point to the STHORM MCAPI directory)
  endif
  include $(P12MCAPI)/examples/rules.mk
endif
