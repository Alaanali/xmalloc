CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = malloc.c malloc_utils.c malloc_chunk.c test.c
OBJS = $(SRCS:.c=.o)
TARGET = malloc_test

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)