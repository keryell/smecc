ROSE_INCLUDE_DIR = /home/lanore/Downloads/compileTree/include
BOOST_CPPFLAGS = -pthread -I/home/lanore/Downloads/boostInstallTree/include
ROSE_LIB_DIR = /home/lanore/Downloads/compileTree/lib
CC                    = gcc
CXX                   = g++
CPPFLAGS              = 
CXXFLAGS              = -g -Wall -Wno-write-strings

ROSE_LIBS = $(ROSE_LIB_DIR)/librose.la

allFiles = parseTest

all: $(allFiles)
	
parseTest: smecyAttribute.o
	@echo [Parser]
	@bison -dt smecyParser.yy
	@echo [Lexer]
	@flex smecyLexer.ll
	@echo [Compiling]
	@libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -o $@ lex.yy.c smecyParser.tab.cc $(ROSE_LIBS) >/dev/null
	
%.o : %.cpp
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<

	
test: parseTest input
	@echo [Testing]
	./parseTest input
	
clean: parseTest
	rm parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c smecyAttribute.o
