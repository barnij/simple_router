CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -D_GNU_SOURCE

NAME = router
SRC = main.c ip_stuff.c str_stuff.c timer.c udp_sender.c
DEPS = entry.h ip_stuff.h str_stuff.h timer.h udp_sender.h
OBJS = main.o ip_stuff.o str_stuff.o timer.o udp_sender.o

YOU :$(SRC) $(NAME)

$(NAME):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)

distclean:
	rm -f $(OBJS) $(NAME)