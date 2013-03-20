Some test cases to verify that some runtime functions behave correctly.


Have a look to the global README.txt to have information about the
configuration of the environment to run them.

Basically the local Makefile can be run with TARGET=MCA or STHORM to
select the MCAPI implementation to compile and run with: respectively the
MultiCore Association reference implementation and the implementation from
CEA for ST STHORM.

Unfortunately, because of the differences in launching threads on the
different targets, these MCAPI example are not compatibles at the
application level (main() and compilation tool-chain). So you have to
select the right target for each example...


Examples to be compiled with "make TARGET=MCA":
===============================================

They use the smecy/runtime/use.mk Makefile.

- producer-consumer:

  A simple 2-thread OpenMP producer-consumer program that indeed does not
  use MCAPI. Just to test the condition variable implementation.

- mca-api-openmp:

  The simple 2-thread OpenMP producer-consumer program that uses MCAPI to
  send data from the producer to the consumer.

- mca-api-producer & mca-api-consumer:

  A simple 2-process application that uses MCAPI to send data from the
  producer to the consumer. You have to launch both programs to have the
  application working

"make TARGET=MCA clean" removes the compiled applications.


Example to be compiled and run with "make TARGET=STHORM":
=========================================================

It uses the $P12MCAPI/examples/rules.mk Makefile. Look at the STHORM MCAPI
documentation in $P12MCAPI/doc/p12mcapi.pdf for more details.

- STHORM_host:

  An application to be compiled & run on STORM simulator with "make run".

  You can choose the target for the host and the accelerator fabric with
  the HOST_TYPE (posix [default] or arm) and FABRIC_TYPE (posix [default]
  or iss) variables.

  For example, to compile and run this application in emulation mode with
  pthreads on your local Linux, use:
  make TARGET=STHORM run

  and to compile and run on the Encore Intruction Set Simulator (ISS), use:
  make TARGET=STHORM FABRIC_TYPE=iss run

The applications are compiled into the "build" directory.

Use
make TARGET=STHORM FABRIC_TYPE=posix distclean
or
make TARGET=STHORM FABRIC_TYPE=iss distclean
to remove the given application.


# Some Emacs stuff:
### Local Variables:
### mode: flyspell
### ispell-local-dictionary: "american"
### End:
