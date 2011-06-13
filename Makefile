allFiles = parseTest

all: $(allFiles)
	
parseTest:
	@echo [Parser]
	@bison -d smecyParser.yy
	@echo [Lexer]
	@flex smecyLexer.ll
	@echo [Compiling]
	@g++ lex.yy.c smecyParser.tab.cc -o parseTest
	
clean:
	rm parseTest smecyParser.tab.cc smecyParser.tab.hh lex.yy.c
