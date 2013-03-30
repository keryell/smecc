SME-C compiler from SMECY project
=================================

smecc is a compiler for a C-based internal representation with #pragma
targeting various parallel embedded systems and accelerators.

There is some documentation on the GitHub wiki at
https://github.com/silkan/smecc/wiki

This is funded by the SMECY ARTEMIS European project http://smecy.eu


Ronan KERYELL at silkan DOT com and others




The main idea of this is to be human while be usable as an internal
representation for various tools.

The organization of the directories:

- code : the compiler

- doc : some documentation on the project, such as basic user guide and a
  specification of the SME-C

- examples : few examples for testing and demos

- report : the MR1 report internship from Vincent Lanore

- ROSE : information about compiling ROSE Compiler for this project

- runtime : the function helpers to adapt to the various software and
  hardware targets


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

Since it is impossible to infer the number of processes used by a generic
SME-C program, we rely on a list of node type and instance to be run.

For example
  for (int i = 0; i < 10; i++) {
#pragma smecy map(PE,i)
...

will need PE #0 up to #9 to run.
A domain is used for PE, a node number for the instance number and group
of ports for a calling instance.


To test that the following implementation works, there are few example in
smecy/runtime/tests


Linux reference implementation of the MCA API:
----------------------------------------------

* Compiling and installing the Linux implementation of the MCA API:

Get the MCAPI_MRAPI_2.0.1_example/mca-2.0.1 from http://www.multicore-association.org/request_mcapi.php?what=MCAPI

To compile on Debian/unstable 64 bit x86, gcc version 4.7.2 (Debian 4.7.2-5):

Compared to the original source distribution, increase some hard-coded
limitations in mca-2.0.1/mrapi/src/mrapi_impl/mrapi_impl_spec.h:
#define MRAPI_MAX_SEMS 200
#define MRAPI_MAX_SHMEMS 200
#define MRAPI_MAX_RMEMS 200
#define MRAPI_MAX_LOCKS 200
#define MRAPI_MAX_SHARED_LOCKS 320


./configure --prefix=/usr/local/mca-api --enable-debug --with-max_domains=6 --with-max_nodes=32

(There are many other default parameters to tweak according to the
application to run. Look by running ./configure --help)

make CFLAGS="-ggdb -Wall"

This CFLAGS is used to remove -Werror to avoid this compilation error:
libtool: compile:  gcc -DHAVE_CONFIG_H -I. -I. -I../../../.. -I ../../../.. -I../../../../common -I../../../../mrapi/include -I../../../../mrapi/src/mrapi_impl -I../../../../mrapi/src/mrapi_impl/sysvr4 -g -Wall -O3 -ggdb -Wall -Werror -MT mrapi.lo -MD -MP -MF .deps/mrapi.Tpo -c ../../mrapi.c  -fPIC -DPIC -o .libs/mrapi.o
../../mrapi.c: In function ‘mrapi_resource_tree_free’:
../../mrapi.c:3166:19: error: variable ‘get_success’ set but not used [-Werror=unused-but-set-variable]

The --enable-debug is to be able to play with mcapi_set_debug_level()

make install


* Using the Linux implementation of the MCA API:

To use it, add the installation bin directory in your PATH, for example
with something like:

PATH=$PATH:/usr/local/mca-api/bin
This makes the mcapi-config command available to give information about
how to use the library for smecc.

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

It looks like in the current MCA API Linux implementation if 2 different
processes communicates with MCAPI, one of the mcapi_finalize() tends to
fail.


ST/CEA STHORM (ex-P2012) implementation of the MCA API:
-------------------------------------------------------

Get the ST STHORM SDK: sthorm-sdk-2013.1-Linux-x86-Install

Get the CEA MCAPI for STHORM P2012-MCAPI-v0.5-2013.1 (21th February 2013)
at least: p12mcapi_V0.5_2013.1.tar.gz

Initialize the STHORM SDK environment, for example with:
export JAVA_HOME=/usr/lib/jvm/default-java
source ~/sthorm-sdk-2013.1/setup.sh
export P12MCAPI=<where you untar-ed p12mcapi_V0.5_2013.1.tar.gz>/P2012-MCAPI-v0.5-2013.1

# Some Emacs stuff:
### Local Variables:
### mode: flyspell
### ispell-local-dictionary: "american"
### End:
