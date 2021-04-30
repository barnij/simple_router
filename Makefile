CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -D_GNU_SOURCE

NAME = router
SRC = main.c ip_stuff.c stuff.c udp_sender.c
DEPS = ip_stuff.h stuff.h udp_sender.h
OBJS = main.o ip_stuff.o stuff.o udp_sender.o

YOU :$(SRC) $(NAME)

$(NAME):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)

distclean:
	rm -f $(OBJS) $(NAME)