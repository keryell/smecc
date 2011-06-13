allFiles = parseTest

all: $(allFiles)
	
parseTest:
	@echo [Parser]
	@bison -dt smecyParser.yy
	@echo [Lexer]
	@flex smecyLexer.ll
	@echo [Compiling]
	@g++ lex.yy.c smecyParser.tab.cc -Wno-write-strings -o parseTest
	
test: parseTest input
	@echo [Testing]
	./parseTest input
	
clean: parseTest
	rm parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c
