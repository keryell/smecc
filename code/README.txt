To compile, try for example:

make ROSE_DIR=/usr/local/rose BOOST_CPPFLAGS=-pthread SMECY_DIR=.

If you want to use Eclipse on this project and you are an Eclipse newbie,
follow the tutorial from Mehdi Amini in
http://www.cri.ensmp.fr/pips/developer_guide.htdoc/developer_guide.pdf
:-)



To run the example, looks at the some rules in the Makefile.


To run a simple test:
make test

To run a test on another file:
./smecyTest --edg:no_warnings -I /hpc/projects/smecy/code/ -c file.C
	for some reason the ".C" extension seems important (bug in the ROSE
	AstRewrite part ?)

To display the AST of the simple test using zgrviewer :
make dot




Old stuff when trying to compile against a Java-enabled ROSE version:

make ROSE_DIR=/usr/local/rose BOOST_CPPFLAGS=-pthread SMECY_DIR=. CXXLDFLAGS=-L/usr/lib/jvm/java-1.5.0-gcj-4.6

LD_LIBRARY_PATH=/usr/lib/jvm/java-6-sun-1.6.0.26/jre/lib/amd64/server:$LD_LIBRARY_PATH
make ROSE_DIR=/usr/local/rose BOOST_CPPFLAGS=-pthread SMECY_DIR=.


# Some Emacs stuff:
### Local Variables:
### mode: flyspell
### ispell-local-dictionary: "american"
### End:
