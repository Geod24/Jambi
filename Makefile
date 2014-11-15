# Written by Mathias LANG
# Swagg added by Arnaud KAPP

NAME=	jambi
vpath %.c src/
SRC=	main.c
OBJDIR=objs
OBJS=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

CFLAGS= -W -Wall

RM=rm -rf
CC=gcc

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re:	fclean all

objs/%.o : %.c
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: re fclean clean all
.NOTPARALLEL: $(NAME)
