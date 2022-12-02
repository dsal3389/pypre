

CC=cc 
C_FLAGS=-Wall 


all:
	$(CC) $(C_FLAGS) -o pypre src/pypre/*.c
