CC = g++
LD = g++
CFLAGS =#
LDFLAGS =#
LLFLAGS= -lpqxx -lpq -lfmt

SRC = main.cpp db.cpp secret.cpp
OBJ=$(SRC:.cpp=.o)
EXE=a.out

all: $(EXE)
	
$(EXE): $(OBJ)
	$(LD) $(LDFLAGS) -o $(EXE) $(OBJ) $(LLFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o a.out
