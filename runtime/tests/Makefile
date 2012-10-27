# Makefile to test some runtime functions

LOCAL_BIN=producer-consumer mca-api-openmp mca-api-producer mca-api-consumer
SMECY_INC=..

# export MCA_INCLUDE=/usr/local/mca-api/include

MCA_INCLUDE:=`mcapi-config --cflags`
MCAPI_LINK:=`mcapi-config --libs`
MRAPI_LINK:=`mrapi-config --libs`
LDLIBS+=$(MCAPI_LINK) $(MRAPI_LINK)

compile: $(LOCAL_BIN)

include ../use.mk
