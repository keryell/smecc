# Makefile to test some runtime functions

# export MCA_INCLUDE=/usr/local/mca-api/include

MCA_INCLUDE:=`mcapi-config --cflags`
MCAPI_LINK:=`mcapi-config --libs`
MRAPI_LINK:=`mrapi-config --libs`
LDLIBS=$(MCAPI_LINK) $(MRAPI_LINK)

CFLAGS=--std=c99 -fopenmp -DSMECY_VERBOSE -g -I.. $(MCA_INCLUDE)
CXXFLAGS=--std=c++0x -fopenmp -DSMECY_VERBOSE -g -I.. $(MCA_INCLUDE)


all: producer-consumer