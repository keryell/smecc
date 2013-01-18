# Where ROSE is installed.
# Default location:
ROSE_DIR			= /usr/local/rose

BOOST_CPPFLAGS		= -pthread

CXX					= g++
LEX					= flex
YACC				= bison
CPPFLAGS			=
# -Wno-write-strings to avoid messages such as:
# smecyParser.tab.cc:1935:35: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
CXXFLAGS			= -g -Wall -Wno-write-strings -std=c++0x
LEXFLAGS			=
YACCFLAGS			= -d
RMFLAGS				= -f
TESTFLAGS			= -smecy -fopenmp

allFiles = smecc smecc_identity smecc_post_processor
smecc_post_processor: LDLIBS += -lboost_regex

# The compilation of the compiler:

all: $(allFiles)

# Put "lex.yy.o smecyParser.tab.o" first to speed up compilation through
# make -jx since it is the longer path:
smecc: lex.yy.o smecyParser.tab.o main.o public.o smecyAttribute.o smecyTranslation.o
	@echo [Linking]
	libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXLDFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -L$(ROSE_DIR)/lib -o $@ $^ -lrose


smecc_identity: main_identity.o
	@echo [Linking]
	libtool --mode=link $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXXLDFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -L$(ROSE_DIR)/lib -o $@ $^ -lrose

%.o : %.cpp %.hpp public.hpp
	@echo [Compiling $<]
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<

%.o : %.cpp public.hpp
	@echo [Compiling $<]
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<

lex.yy.o : lex.yy.c smecyParser.tab.cc public.hpp
	@echo [Compiling $<]
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<

smecyParser.tab.o : smecyParser.tab.cc public.hpp
	@echo [Compiling $<]
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ROSE_DIR)/include $(BOOST_CPPFLAGS) -c -o $@ ./$<

smecyParser.tab.cc : smecyParser.yy public.hpp
	@echo [Parser $<]
	$(YACC) $(YACCFLAGS) $<

lex.yy.c : smecyLexer.ll public.hpp
	@echo [Lexer $<]
	$(LEX) $(LEXFLAGS) $<

# Running some examples:

test: smecc input.C
	@echo [Testing $<]
	./smecc $(TESTFLAGS) -c input.C

testFortran: smecc input.f
	@echo [Testing $<]
	./smecc -c input.f

testCompile: smecc input.C
	@echo [Testing $<]
	./smecc $(TESTFLAGS) input.C -o compiledTest

dot: test
	apps/zgrviewer/run.sh ./input.C.dot

dotFortran: testFortran
	apps/zgrviewer/run.sh ./input.f.dot


clean:
	rm $(RMFLAGS) *.o smecyParser.tab.cc smecyParser.tab.hh lex.yy.c rose_* input.C.* compiledTest input.f.* *~
