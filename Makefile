# Written by Mathias LANG
# Swagg added by Arnaud KAPP

NAME=	jambi
vpath %.c src/
SRC=	parser.c \
	read_file.c \
	magic.c
OBJDIR=objs
OBJS=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Main source file.
MAINSRC=main.c
MAINOBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(MAINSRC))
# For testing purpose.
TESTSRC=test.c
TESTOBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(TESTSRC))

CFLAGS= -W -Wall

RM=rm -rf
CC=gcc

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS) $(MAINOBJ)
	$(CC) $(CFLAGS) $(OBJS) $(MAINOBJ) -o $(NAME)

test:	$(OBJDIR) $(OBJS) $(TESTOBJ)
	$(CC) $(CFLAGS) $(OBJS) $(TESTOBJ) -o $(NAME)

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
