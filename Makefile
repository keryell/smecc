ROSE_DIR			= /hpc/projects/smecy/libs/roseInstall
SMECY_DIR			= /hpc/projects/smecy/
BOOST_CPPFLAGS		= -pthread -I/hpc/projects/smecy/libs/boostInstall/include
ROSE_LIBS			= $(ROSE_DIR)/lib/librose.la
CXX					= g++
LEX					= flex
YACC				= bison
CPPFLAGS			=
CXXFLAGS			= -g -Wall -Wno-write-strings
LEXFLAGS			=
YACCFLAGS			= -d
RMFLAGS				= -f
TESTFLAGS			= -rose:openmp:ast_only --edg:no_warnings #ast_only, lowering, parse_only

allFiles = smecyTest

all: $(allFiles)

smecyTest: main.o smecyAttribute.o smecyTranslation.o lex.yy.o smecyParser.tab.o
	@echo [Linking]
	@libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -o $@ $^ $(ROSE_LIBS) >/dev/null
		
%.o : %.cpp %.h public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
%.o : %.cpp public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
lex.yy.o : lex.yy.c smecyParser.tab.cc public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
smecyParser.tab.o : smecyParser.tab.cc public.h
	@echo [Compiling $<]
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<
	
smecyParser.tab.cc : smecyParser.yy public.h
	@echo [Parser $<]
	@$(YACC) $(YACCFLAGS) $<
	
lex.yy.c : smecyLexer.ll public.h
	@echo [Lexer $<]
	@$(LEX) $(LEXFLAGS) $<
	
test: smecyTest input.C
	@echo [Testing $<]
	@./smecyTest $(TESTFLAGS) -c input.C

dot: test
	@$(SMECY_DIR)/apps/zgrviewer/run.sh ./input.C.dot

clean:
	@rm $(RMFLAGS) *.o smecyTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c rose_* input.C.*
	
backup: clean
	@mkdir ~/stage/codeBackup/`date +"%m%d%H%M"`smecy/
	@cp -r * ~/stage/codeBackup/`date +"%m%d%H%M"`smecy/
