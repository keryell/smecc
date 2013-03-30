SME-C compilation with smecc:
=============================
Have a look to the global README.txt to have information about the
configuration of the environment to run them.

Basically the local Makefile can be run with TARGET=MCA or STHORM to
select the MCAPI implementation to compile and run with: respectively the
MultiCore Association reference implementation and the implementation from
CEA for ST STHORM.



The use.mk Makefile can produce basically many different targets

If we have for example a <local>.c C99 or <local>.C C++ source file in the
directory, we can produce:

- <local> is the sequential binary;

- <local>.E is the source code post-processed with CPP;

- smecy_<local>.c and smecy_<local>.C are the program generated to be run
  on the host;

- smecy_<local>.E is the source code of the OpenMP emulation
  post-processed with CPP;

- smecy_<local> is a binary that simulate with OpenMP the SME-C program,
  with both the host side and accelerator side. The host and accelerated
  part communicate through pointer in the same global memory;

- accel_smecy_<local>.c and accel_smecy_<local>.C are the accelerator side
  code, with the code that dispatches the request from the host;


- smecy_<local>_host is the MCAPI executable to be executed on the host;

- smecy_<local>_host.E is the MCAPI source code to be executed on the
  host after post-processing with CPP;

- accel_smecy_<local>_fabric is the MCAPI executable code to be executed
  on the accerator;

- accel_smecy_<local>_fabric.E is the MCAPI source code to be executed on the
  accerator after post-processing with CPP.



This Makefile defines also some convenient global targets:

- clean: removed the generated files;

- raw: generate the unchanged SME-C program. Note that since it is
  based on OpenMP, it may be a parallel program;

- openmp: generate binary with OpenMP emulation of host and accelerator
  sides

- mcapi: generate both accelerator and host binary using MCAPI

- smecy: generate the sources to be run on the host with MCAPI or on both
  host and accelerator with OpenMP emulation;

- accel_smecy: generate the sources to be run on the accelerator with
  MCAPI;

- bin: generate all possible kinds of binaries.


By default it is compiled in verbose and tracing mode.
Add "SMECY_DEBUG=" to the make line to cancel these modes.


ST STHORM platform:
-------------------

It uses the $P12MCAPI/examples/rules.mk Makefile. Look at the STHORM MCAPI
documentation in $P12MCAPI/doc/p12mcapi.pdf for more details.
You have to initialize the STHORM SDK environment, for example with:
export JAVA_HOME=/usr/lib/jvm/default-java
source ~/sthorm-sdk-2013.1/setup.sh
export P12MCAPI=<where you untar-ed p12mcapi_V0.5_2013.1.tar.gz>/P2012-MCAPI-v0.5-2013.1

Then you have also to set the variable:


You can choose the target for the host and the accelerator fabric with
the HOST_TYPE (posix [default] or arm) and FABRIC_TYPE (posix [default]
or iss) variables.

For example, to compile and run this application in emulation mode with
pthreads on your local Linux, use:
make TARGET=STHORM run_smecy_<local>_host

and to compile and run on the Encore Intruction Set Simulator (ISS), use:
make TARGET=STHORM FABRIC_TYPE=iss run_smecy_<local>_host

To run it in the debugger, use:
make TARGET=STHORM FABRIC_TYPE=iss debug_smecy_<local>_host

The applications are compiled into the "build" directory.

Use
make TARGET=STHORM FABRIC_TYPE=posix distclean
or
make TARGET=STHORM FABRIC_TYPE=iss distclean
to remove the given application.
