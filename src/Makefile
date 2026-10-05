# Compiler and flags
CFLAGS = -std=c11 -Wall -Wextra -g -O2 
CPPFLAGS = -I../include -DDEBUG
LDFLAGS =

# Targets
all: sudoku 

sudoku: sudoku.o grid.o colors.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Object rules
grid.o: grid.c ../include/grid.h ../include/colors.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

colors.o: colors.c ../include/colors.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

sudoku.o: sudoku.c sudoku.h ../include/grid.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

# Clean up
clean:
	@rm -f *.o *.d sudoku

# Help
help:
	@echo "Makefile targets:"
	@echo "  all    - Build sudoku and colors"
	@echo "  clean  - Remove all build files"
	@echo "  help   - Show this help message"

.PHONY: all clean help
