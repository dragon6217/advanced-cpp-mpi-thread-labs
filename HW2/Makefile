CC=g++
CFLAG=-Wall -Werror -g

SRC=$(wildcard *.cc)
HDR=$(wildcard *.h) $(wildcard *.hpp)
OBJ=$(SRC:.cc=.o)
EXE=vector

.PHONY: clean

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ)

%.o: %.cc $(HDR)
	$(CC) $(CFLAG) -o $@ -c $<

clean:
	rm -f $(OBJ) $(EXE)

