ROSE_INCLUDE_DIR	= /hpc/projects/smecy/libs/roseInstall/include
BOOST_CPPFLAGS		= -pthread -I/hpc/projects/smecy/libs/boostInstall/include
ROSE_LIB_DIR 		= /hpc/projects/smecy/libs/roseInstall/lib
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
TESTFLAGS			= -rose:openmp:parse_only #ast_only, lowering, parse_only

allFiles = parseTest

all: $(allFiles)

parseTest: main.o smecyAttribute.o smecyAstConstruction.o lex.yy.o smecyParser.tab.o sgSmecyNodes.o
	@echo [Linking]
	@libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -o $@ $^ $(ROSE_LIBS) >/dev/null
		
%.o : %.cpp %.h public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
%.o : %.cpp public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
lex.yy.o : lex.yy.c smecyParser.tab.cc public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
smecyParser.tab.o : smecyParser.tab.cc public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_INCLUDE_DIR) $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
smecyParser.tab.cc : smecyParser.yy public.h
	@echo [Parser $<]
	@$(YACC) $(YACCFLAGS) $<
	
lex.yy.c : smecyLexer.ll public.h
	@echo [Lexer $<]
	@$(LEX) $(LEXFLAGS) $<
	
test: parseTest input.cpp
	@echo [Testing $<]
	@./parseTest $(TESTFLAGS) -c input.cpp

clean:
	@rm $(RMFLAGS) *.o parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c rose_input.cpp input.cpp.*
	
backup: clean
	@mkdir ~/stage/codeBackup/`date +"%m%d%H%M"`smecy/
	@cp * ~/stage/codeBackup/`date +"%m%d%H%M"`smecy/
