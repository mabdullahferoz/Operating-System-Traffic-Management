CC = gcc
CFLAGS = -Wall -g -pthread
LIBS = -lrt

SRCS = main.c traffic_logic.c ipc_manager.c utils.c
OBJS = $(SRCS:.c=.o)
TARGET = traffic_system

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
