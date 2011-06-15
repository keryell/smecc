ROSE_INCLUDE_DIR	= /home/lanore/Downloads/compileTree/include
BOOST_CPPFLAGS		= -pthread -I/home/lanore/Downloads/boostInstallTree/include
ROSE_LIB_DIR 		= /home/lanore/Downloads/compileTree/lib
CXX					= g++
CPPFLAGS			= 
CXXFLAGS			= -g -Wall -Wno-write-strings
ROSE_LIBS			= $(ROSE_LIB_DIR)/librose.la

allFiles = parseTest

#TODO add dependencies with the .hs
all: $(allFiles)

parseTest: main.cpp smecyAttribute.o lex.yy.o smecyParser.tab.o	
	@echo [Linking]
	@libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -o $@ $^ $(ROSE_LIBS) >/dev/null
	
smecyAttribute.cpp : smecyAttribute.h
	
%.o : %.cpp
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
lex.yy.o : lex.yy.c smecyParser.tab.cc
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
smecyParser.tab.o : smecyParser.tab.cc
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
smecyParser.tab.cc : smecyParser.yy
	@echo [Parser $<]
	@bison -d smecyParser.yy
	
lex.yy.c : smecyLexer.ll
	@echo [Lexer $<]
	@flex smecyLexer.ll
	
test: parseTest input
	@echo [Testing]
	@./parseTest input

#TODO rewrite to allow cleaning partial build
clean:
	rm *.o
	rm parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c
