.PHONY: all clean

SRC = read_boot.c read_mbr.c read_filesystem.c read_file.c recover_files.c
BIN = $(SRC:.c=)

clean:
	rm -f $(BIN) $(OBJ)

all: 
	gcc read_boot.c -o read_boot
	gcc read_mbr.c -o  read_mbr
	gcc read_filesystem.c -o read_filesystem
	gcc read_file.c -o read_file
	gcc recover_files.c -o recover_files
