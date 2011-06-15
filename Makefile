ROSE_INCLUDE_DIR	= /home/lanore/Downloads/compileTree/include
BOOST_CPPFLAGS		= -pthread -I/home/lanore/Downloads/boostInstallTree/include
ROSE_LIB_DIR 		= /home/lanore/Downloads/compileTree/lib
ROSE_LIBS			= $(ROSE_LIB_DIR)/librose.la
CXX					= g++
LEX					= flex
YACC				= bison
CPPFLAGS			= 
CXXFLAGS			= -g -Wall -Wno-write-strings
RMFLAGS				=
LEXFLAGS			=
YACCFLAGS			= -d
RMFLAGS				= -f

allFiles = parseTest

all: $(allFiles)

parseTest: main.o smecyAttribute.o smecyAstConstruction.o lex.yy.o smecyParser.tab.o	
	@echo [Linking]
	@libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -o $@ $^ $(ROSE_LIBS) >/dev/null
		
%.o : %.cpp %.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
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
	@$(YACC) $(YACCFLAGS) $<
	
lex.yy.c : smecyLexer.ll
	@echo [Lexer $<]
	@$(LEX) $(LEXFLAGS) $<
	
test: parseTest input.cpp
	@echo [Testing $<]
	@./parseTest -c input.cpp

clean:
	rm $(RMFLAGS) *.o parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c rose_input.cpp
	
backup: clean
	cp * ~/stage/codeBackup/
