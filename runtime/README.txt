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
