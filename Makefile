ROSE_INCLUDE_DIR	= /home/lanore/Downloads/compileTree/include
BOOST_CPPFLAGS		= -pthread -I/home/lanore/Downloads/boostInstallTree/include
ROSE_LIB_DIR 		= /home/lanore/Downloads/compileTree/lib
CXX					= g++
CPPFLAGS			= 
CXXFLAGS			= -g -Wall -Wno-write-strings
ROSE_LIBS			= $(ROSE_LIB_DIR)/librose.la

allFiles = parseTest
parserFiles = main.cpp lex.yy.o smecyParser.tab.o smecyAttribute.o

all: $(allFiles)

#TODO divide in different steps to avoid whole recompilation each time
parseTest: smecyAttribute.o lex.yy.o smecyParser.tab.o	
	@echo [Linking]
	@libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -o $@ $(parserFiles) $(ROSE_LIBS) >/dev/null
	
%.o : %.cpp
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
lex.yy.o : lex.yy.c smecyParser.tab.cc
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./lex.yy.c
	
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

#TODO write less weird clean rule
clean:
	rm *.o
	rm parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c
