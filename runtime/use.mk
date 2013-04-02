# A Makefile to ease compilation of SME-C with smecc
# SMECY ARTEMIS European project
# Ronan Keryell @ silkan.com

# To run smecc with ROSE in verbose mode, try:
# make ROSE_FLAGS="-rose:verbose 3"

# Think to use a make xyz.E to produce a preprocessed output to understand
# what is really compiled

# Where is smecy.h:
# You have to set this before using this Makefile
#SMECY_INC=../runtime

# To put more flags, use make MORE_FLAGS=...

# To skip debug, use "make SMECY_DEBUG="
SMECY_DEBUG=-DSMECY_VERBOSE -DSMECY_MCAPI_CHECK_TRACE
# You can add for internal MCAPI tracing
# -DSMECY_MCA_API_DEBUG_LEVEL=1
# up to 7

ifneq ($(TARGET),STHORM)
  # By default use MCA API reference implementation
  MCA_INCLUDE?=`mcapi-config --cflags`
  MCAPI_LINK?=`mcapi-config --libs`
  MRAPI_LINK?=`mrapi-config --libs`
endif

# Add some flags to previously ones, if any:
SMECY_FLAGS+=$(SMECY_DEBUG) -I$(SMECY_INC)
CFLAGS+=--std=c99 -fopenmp -g3 -gdwarf-4 $(SMECY_FLAGS) $(MCA_INCLUDE) $(MORE_FLAGS)

# More flags to compile with MCAPI on the host side
CFLAGS_MCAPI=-DSMECY_MCAPI -DSMECY_MCAPI_HOST
# More flags to compile with MCAPI on the accelerator side
CFLAGS_MCAPI_ACCEL=-DSMECY_MCAPI

LDFLAGS+=-fopenmp
CXXFLAGS+=--std=c++0x -fopenmp -g3 -gdwarf-4 $(SMECY_FLAGS) $(MCA_INCLUDE) $(MORE_FLAGS)
LDLIBS+=$(MCAPI_LINK) $(MRAPI_LINK)


# Have an explicit list since we have some program with multiple
# compilation units:

# smecc generate file beginning with "smecy_" and "smecy_accel_" for the
# host and accelerator sides respectively:
LOCAL_SMECY=$(addprefix smecy_,$(LOCAL_SRC))
LOCAL_ACCEL_SMECY=$(addprefix accel_smecy_,$(LOCAL_SRC))
LOCAL_SMECY_MCAPI=$(patsubst %.c,%_host.c,$(patsubst %.C,%_host.C,$(LOCAL_SMECY)))
LOCAL_ACCEL_SMECY_MCAPI=$(patsubst %.c,%_fabric.c,$(patsubst %.C,%_fabric.C,$(LOCAL_ACCEL_SMECY)))
LOCAL_DOT=$(addsuffix .dot,$(LOCAL_SRC))
LOCAL_PDF=$(addsuffix .pdf,$(LOCAL_SRC))

# Remove .c and .C extension to have executable names:
#LOCAL_ROSE_BIN:=$(LOCAL_ROSE:.c=)
#LOCAL_ROSE_BIN:=$(LOCAL_ROSE_BIN:.C=)


LOCAL_SMECY_BIN=$(addprefix smecy_,$(LOCAL_BIN))
LOCAL_SMECY_MCAPI_BIN=$(addsuffix _host,$(LOCAL_SMECY_BIN))
LOCAL_ACCEL_SMECY_MCAPI_BIN=$(addprefix accel_,$(addsuffix _fabric,$(LOCAL_SMECY_BIN)))

# Some high-level convenience targets:

# By default, display the doc after figuring out where we are:
THIS_MAKEFILE_DIR:=$(abspath $(shell dirname $(lastword $(MAKEFILE_LIST))))
readme:
	cat $(THIS_MAKEFILE_DIR)/README.txt

raw: $(LOCAL_BIN)

openmp: $(LOCAL_SMECY_BIN)

mcapi_host: $(LOCAL_SMECY_MCAPI_BIN)

mcapi_fabric: $(LOCAL_ACCEL_SMECY_MCAPI_BIN)

mcapi: mcapi_host mcapi_fabric

smecy: $(LOCAL_SMECY)

accel_smecy: $(LOCAL_ACCEL_SMECY)

bin: raw openmp mcapi

# Use :: so that a user of this Makefile can extend this rule
clean::
	rm -rf $(LOCAL_SMECY) $(LOCAL_ACCEL_SMECY) $(LOCAL_SMECY_MCAPI)	\
	   $(LOCAL_ACCEL_SMECY_MCAPI) $(LOCAL_BIN) $(LOCAL_SMECY_BIN)	\
	   $(LOCAL_SMECY_MCAPI_BIN) $(LOCAL_ACCEL_SMECY_MCAPI_BIN)	\
	   $(LOCAL_DOT) $(LOCAL_PDF) rose_transformation_* *.o *.E	\
	   *-smecy_dispatch *.smecc_pp \
	   build


# Detailed targets

# Produce a specialized MCAPI expansion for fabric side
# accel_smecy_... source files:
accel_%: CFLAGS += $(CFLAGS_MCAPI_ACCEL)
# Produce a specialized MCAPI expansion for host side smecy_... source files:
%_host %_host.E: CFLAGS += $(CFLAGS_MCAPI)

# Generate the normal and MCAPI _host versions with the same rule
smecy_%.C: %.C
	smecc -smecy -rose:skipfinalCompileStep $(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.C $@

smecy_%.c: %.c
	smecc -smecy --std=c99 -rose:C99_only \
		-rose:skipfinalCompileStep -rose:C_output_language \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.c $@

smecy_%_host.c: smecy_%.c
	# They are the same file indeed
	cp -a $< $@

# Generate the normal and MCAPI _fabric (accelerator) versions with the
# same rule
accel_smecy_%.C: %.C
	smecc -smecy -smecy-accel -rose:skipfinalCompileStep \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.C $@
	# Post process the output:
	smecc_post_processor $@
	# Append the automatically generated file (if any) with the
	# dispatch system on the PEs to the post-processed version back
	# into the target file:
	if [ -r $*.C-smecy_dispatch ]; then \
	   cat $@.smecc_pp $*.C-smecy_dispatch > $@ ; \
	else \
	   mv $@.smecc_pp $@ ; \
	fi

smecy_%_host.C: smecy_%.C
	# They are the same file indeed
	cp -a $< $@

accel_smecy_%_fabric.C: accel_smecy_%.C
	# They are the same file indeed
	cp -a $< $@

accel_smecy_%.c: %.c
	smecc -smecy -smecy-accel --std=c99 -rose:C99_only \
		-rose:skipfinalCompileStep -rose:C_output_language \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.c $@
	# Post process the output:
	smecc_post_processor $@
	# Append the automatically generated file (if any) with the
	# dispatch system on the PEs to the post-processed version back
	# into the target file:
	if [ -r $*.c-smecy_dispatch ]; then \
	   cat $@.smecc_pp $*.c-smecy_dispatch > $@ ; \
	else \
	   mv $@.smecc_pp $@ ; \
	fi

accel_smecy_%_fabric.c: accel_smecy_%.c
	# They are the same file indeed
	cp -a $< $@

# Beautifying the ouput:
%-format.c: %.E
	grep -v '^#' $< > $@ ; astyle $@


ifeq ($(TARGET),STHORM)
  ifndef P12MCAPI
    $(error You have to set the P12MCAPI environment variable to point to the STHORM MCAPI directory and source the SDK setup.sh as explained in the README.txt)
  endif

  # To use the Makefile provided with STHORM MCAPI:
  MAKE_FOR_STHORM=$(MAKE) -f $(P12MCAPI)/examples/rules.mk $(MORE_P12MCAPI)

  # The STxP70 compiler has an option to select C99 with a '-' less
  # than with GCC:
  CFLAGS:=$(subst --std=c99,-std=c99,$(CFLAGS))
  # There is no OpenMP support on the accelerator:
  CFLAGS:=$(filter-out -fopenmp,$(CFLAGS))

  # The variables we want to communicate to the MCAPI STHORM makefile:
  export FABRIC_CFLAGS:=$(CFLAGS) -DMCAPI_STHORM $(CFLAGS_MCAPI_ACCEL)
  export CFLAGS+=-DMCAPI_STHORM $(CFLAGS_MCAPI)
  export CXXFLAGS+=-DMCAPI_STHORM
  export LDFLAGS
  export LDLIBS
  # The name of the library our runtime expect to load on the STHORM fabric:
  export FABRIC_LIB=smecy_accel_fabric
  # By default, forward any target to the MCAPI STHORM Makefile, for
  # example for the distclean target:
  .DEFAULT:
	$(MAKE_FOR_STHORM) $(MAKEFLAGS) $(MAKECMDGOALS)

  # On STHORM, we invoke the MCAPI STHORM makefiles to compile and run the
  # application by setting the variables defined in the STHORM... variable:
  run_%_host:
	# Since we have to build some dependences with different variable
	# values for host and fabric side, use sub-make to build them,
	# after evaluation the environment variables to get the source
	# lists. Since the variable values are available only inside a
	# process, use "sh -c", with simple quotes to avoid evaluation of
	# the variable at the top level:
	$(STHORM_$*_host) sh -c '$(MAKE) $(MAKEFLAGS) $$HOST_SRC'
	$(STHORM_$*_host) sh -c '$(MAKE) $(MAKEFLAGS) $$FABRIC_SRC'
	# Rely on the MCAPI STHORM makefile to compile for and run on the
	# target:
	$(MAKE_FOR_STHORM) $(STHORM_$*_host) run

  debug_%_host:
	# Think to do a "handle SIGUSR1 noprint nostop" in GDB
	$(MAKE_FOR_STHORM) $(STHORM_$*_host) debug


  # Produce a CPP output to help debugging
  accel_%.E: accel_%.[cC]
	# Keep comments in the output
	$(CPP) -CC $(FABRIC_CFLAGS) -I$(P12MCAPI)/include $< > $@

else

  run_%: %
	./$<

endif

# Produce a CPP output to help debugging
%.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(CFLAGS) $< > $@

%.E: %.x
	$(CPP) -CC $(CFLAGS) $< > $@
