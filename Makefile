ROSE_DIR			= /hpc/projects/smecy/libs/roseInstall
CODE_DIR			= /hpc/projects/smecy/code
ROSE_INCLUDE_DIR	= $(ROSE_DIR)/include
BOOST_CPPFLAGS		= -pthread -I/hpc/projects/smecy/libs/boostInstall/include
ROSE_LIB_DIR 		= $(ROSE_DIR)/lib
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
ROSETTA_DIR			= /hpc/projects/smecy/libs/rose-0.9.5a-14690/src/ROSETTA
ROSETTAFILES		= $(ROSETTA_DIR)/astNodeList $(ROSETTA_DIR)/src/statement.C $(ROSETTA_DIR)/src/node.C $(ROSETTA_DIR)/Grammar/Statement.code

allFiles = parseTest

all: $(allFiles)

parseTest: main.o smecyAttribute.o smecyAstConstruction.o lex.yy.o smecyParser.tab.o
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

dot: test
	@/home/lanore/Downloads/zgrviewer/run.sh ./input.cpp.dot

rose-clean:
	@make -C $(ROSE_DIR) clean

rose:
	@cp rosettaBackup/astNodeList $(ROSETTA_DIR)/astNodeList
	@cp rosettaBackup/statement.C $(ROSETTA_DIR)/src/statement.C
	@cp rosettaBackup/node.C $(ROSETTA_DIR)/src/node.C
	@cp rosettaBackup/Statement.code $(ROSETTA_DIR)/Grammar/Statement.code
	@make -C $(ROSE_DIR)
	@make -C $(ROSE_DIR) install
	
rose6:
	@cp rosettaBackup/astNodeList $(ROSETTA_DIR)/astNodeList
	@cp rosettaBackup/statement.C $(ROSETTA_DIR)/src/statement.C
	@cp rosettaBackup/node.C $(ROSETTA_DIR)/src/node.C
	@cp rosettaBackup/Statement.code $(ROSETTA_DIR)/Grammar/Statement.code
	@make -C $(ROSE_DIR) -j6
	@make -C $(ROSE_DIR) install

clean:
	@rm $(RMFLAGS) *.o parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c rose_input.cpp input.cpp.*
	
backup: clean
	@mkdir ~/stage/codeBackup/`date +"%m%d%H%M"`smecy/
	@cp -r * ~/stage/codeBackup/`date +"%m%d%H%M"`smecy/
