Some information about how we get ROSE Compiler running on Debian/unstable
for the smecc compiler from the SMECY project.

Ronan.Keryell@wild-systems.com



http://www.rosecompiler.org/ROSE_InstallationInstructions.pdf


Cannot have Java compilation running:
#export LD_LIBRARY_PATH=/usr/lib/jvm/java-6-sun-1.6.0.26/jre/lib/amd64/server:$LD_LIBRARY_PATH


Install old gcc-4.4 and g++-4.4 to be able to use precompiled version of EDG.

On Debian/unstable:
aptitude install libboost-all-dev libhpdf-dev libgcrypt11-dev


# Create the default installation directory:
sudo mkdir /usr/local/rose
# Adapt here: :-)
sudo chown keryell.keryell /usr/local/rose


# Make the old compiler current to be used during ROSE compilation:
sudo rm /usr/bin/gcc
sudo ln -s gcc-4.4 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s g++-4.4 /usr/bin/g++
sudo rm /usr/bin/cpp
sudo ln -s cpp-4.4 /usr/bin/cpp

With a distribution, this is simple :
tar zxvf rose-0.9.5a-without-EDG-19065.tar.gz

With git :
git clone http://www.rosecompiler.org/rose.git
cd rose
./build



# Then begin the compilation somewhere:
mkdir compileTree
cd compileTree
#../rose-0.9.5a-19065/configure --prefix=/usr/local/rose --with-boost=/usr
# --disable-binary-analysis-tests --enable-only-c --without-java


# Compile with --disable-projects-directory --disable-tests-directory
# --disable-tutorial-directory because it is too slow
../rose/configure --prefix=/usr/local/rose --with-boost=/usr --disable-binary-analysis-tests --disable-projects-directory --disable-tests-directory --disable-tutorial-directory --without-java --disable-cuda --disable-fortran --disable-opencl --disable-php --disable-ppl --disable-cloog --disable-scoplib --disable-candle --disable-python --disable-xmltest

make -j4

make install

To avoid recompiling everything when debugging ROSE core, use
make -C src
and
make install-core



Then, to have C++11 support and a compiler still working with CUDA 4.2 :
sudo rm /usr/bin/gcc
sudo ln -s gcc-4.6 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s g++-4.6 /usr/bin/g++
sudo rm /usr/bin/cpp
sudo ln -s cpp-4.6 /usr/bin/cpp

To be back to the last compiler instead:
sudo rm /usr/bin/gcc
sudo ln -s gcc-4.7 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s g++-4.7 /usr/bin/g++
sudo rm /usr/bin/cpp
sudo ln -s cpp-4.7 /usr/bin/cpp





For Ubuntu 12.04 :

mkdir compileTree
cd compileTree

aptitude install libboost-all-dev libhpdf-dev libgcrypt11-dev

# Make the old compiler current to be used during ROSE compilation:
sudo rm /usr/bin/gcc
sudo ln -s gcc-4.4 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s g++-4.4 /usr/bin/g++
sudo rm /usr/bin/cpp
sudo ln -s cpp-4.4 /usr/bin/cpp

../rose/configure --prefix=/usr/local/rose --with-boost=/usr --disable-binary-analysis-tests --disable-projects-directory --disable-tests-directory --disable-tutorial-directory --without-java --disable-cuda --disable-fortran --disable-opencl --disable-php --disable-ppl --disable-cloog --disable-scoplib --disable-candle --disable-python --disable-xmltest

make -j4
make install

# To get back a more modern compiler:
sudo rm /usr/bin/gcc
sudo ln -s gcc-4.6 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s g++-4.6 /usr/bin/g++
sudo rm /usr/bin/cpp
sudo ln -s cpp-4.6 /usr/bin/cpp




Bug report & RFE:

On Debian/unstable x86_64

This should work:
CC=gcc-4.4 CXX=g++-4.4 ../rose-0.9.5a-14690/configure

ERROR: Not able to recognize compiler gcc-4.4
ERROR: Usage is create_system_headers compiler target_dir src_dir
error_code = 1
Error in ../rose-0.9.5a-14690/config/create_system_headers: nonzero exit code returned to caller error_code = 1

Using back-end C++ compiler = "g++-4.4" compiler name = "g++-4.4" for processing of unparsed source files from ROSE preprocessors.
we reached here for some reason (cannot identify back-end C++ compiler "g++-4.4")

I suggest testing only that $CC and $CXX start with their respective basic names...

Work around:
sudo rm /usr/bin/gcc
sudo ln -s gcc-4.4 /usr/bin/gcc
sudo rm /usr/bin/g++
sudo ln -s g++-4.4 /usr/bin/g++
sudo rm /usr/bin/cpp
sudo ln -s cpp-4.4 /usr/bin/cpp

By comparison F77=gfortran-4.4 works fine, but using gfortran 4.6 by default works too...

By the way, the section 0.1.8 "Running GNU Make in Parallel" from the installation manual may state that each g++ process may use a lot of memory (700 MB on my computer), so make -j8 will eat for example around 6 GB to run. That hang my laptop... :-) But may also kill a server with make -j64 :-)
So there is a trade-off to find to use a maximum of CPU without swapping...
It is between minutes (9 on my 4-core 4 GB laptop) and hours or days... :-/

This manual does not talk about libhpdf that is needed to generate PDF
files (issue appears when doing a make install in ROSE). libhpdf-dev is
the packet to install on Debian.


# Some Emacs stuff:
### Local Variables:
### mode: flyspell
### ispell-local-dictionary: "american"
### End:
