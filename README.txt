SME-C compiler from SMECY project
=================================

smecc is a compiler for a C-based internal representation with #pragma
targeting various parallel embedded systems and accelerators.

http://smecy.eu

Ronan KERYELL at Wild-Systems DOT com and others




The main idea of this is to be human while be usable as an internal
representation for various tools.

The organization of the directories:

- code : the compiler

- doc : basic user guide

- examples : few examples for testing and demos

- report : the MR1 report internship from Vincent Lanore

- ROSE : information about compiling ROSE Compiler for this project

- runtime : the function helpers to adapt to the various software and
  hardware targets

- unknown : to be still sorted...


Currently there is no outlining of some mapped functions to other files
since it seems that ROSE Compiler does not support it.


OpenMP back-end:
================

This is an OpenMP emulation of the SME-C #pragma, for example with
parallel sections used to implement pipelined loop streaming.


MCA API back-end:
=================

The MCA API are used to map some SME-C remote computation nodes to MCA
nodes.

The data transfers between the different computing nodes are done by using
unidirectional connection-oriented packet channels between endpoints on
the different nodes.

Compiling the Linux implementation of the MCA API:

MCAPI_MRAPI_2.0.1_example/mca-2.0.1
To compile on Debian/unstable 64 bit x86, gcc version 4.7.1 (Debian 4.7.1-5):

./configure --prefix=/usr/local/mca-api --enable-debug
make CFLAGS="-ggdb -Wall"

This CFLAGS is used to remove -Werror to avoid this compilation error:
libtool: compile:  gcc -DHAVE_CONFIG_H -I. -I. -I../../../.. -I ../../../.. -I../../../../common -I../../../../mrapi/include -I../../../../mrapi/src/mrapi_impl -I../../../../mrapi/src/mrapi_impl/sysvr4 -g -Wall -O3 -ggdb -Wall -Werror -MT mrapi.lo -MD -MP -MF .deps/mrapi.Tpo -c ../../mrapi.c  -fPIC -DPIC -o .libs/mrapi.o
../../mrapi.c: In function ‘mrapi_resource_tree_free’:
../../mrapi.c:3166:19: error: variable ‘get_success’ set but not used [-Werror=unused-but-set-variable]

The --enable-debug is to be able to play with mcapi_set_debug_level()

The Linux implementation of the MCA API is based on shared memory and
semaphore IPC and is a little bit touchy when things go wrong, with
messages such as:
mrapi_initialize failed status=UNKNOWN ERROR
API call fails with error MCAPI_ERR_NODE_INITFAILED

Shared memory segments and semaphores may stay in a wrong state,
preventing the runtime to run again. These IPC objects can be displayed
with ipcs and from my experience are the ones with a non null key. Then
you can use
ipcrm -M ... -S ...

to remove the objects from their keys. It is more practical to use the
keys instead of the IDs since they keeps their value from one
run... errr... failure to the other one.


# Some Emacs stuff:
### Local Variables:
### mode: flyspell
### ispell-local-dictionary: "american"
### End:
