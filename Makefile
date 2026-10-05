# Variables
EXE = sudoku

# Rules and targets
all:build

build:
	cd src && $(MAKE)
	@cp -f src/$(EXE) ./

clean:
	cd src && $(MAKE) clean
	@rm -f $(EXE)

help:
	@echo "Makefile targets:"
	@echo "all: Run the whole build of sudoku;"
	@echo "build: Build the sudoku executable;"
	@echo "clean: Remove all files produced by the compilation;"
	@echo "help: Display this help."

.PHONY: all build clean help