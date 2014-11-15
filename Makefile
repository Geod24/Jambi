# Written by Mathias LANG
# Swagg added by Arnaud KAPP

NAME=	jambi
SHELLCODE= shellcode
vpath %.c src/
SRC=	parser.c \
	read_file.c \
	magic.c \
	utils.c
OBJDIR=objs
OBJS=$(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Main source file.
MAINSRC=main.c
MAINOBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(MAINSRC))
# For testing purpose.
TESTSRC=test.c
TESTOBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(TESTSRC))
# Shellcode
SHELLSRC=	shellcode.c \
		utils.c \
		read_file.c
SHELLOBJ=$(patsubst %.c,$(OBJDIR)/%.o,$(SHELLSRC))


CFLAGS= -W -Wall -std=c99

RM=rm -rf
CC=x86_64-w64-mingw32-gcc

all: $(NAME) $(SHELLCODE)

$(NAME): $(OBJDIR) $(OBJS) $(MAINOBJ)
	$(CC) $(CFLAGS) $(OBJS) $(MAINOBJ) -o $(NAME)

test:	$(OBJDIR) $(OBJS) $(TESTOBJ)
	$(CC) $(CFLAGS) $(OBJS) $(TESTOBJ) -o $(NAME)

# http://stackoverflow.com/questions/3797310/what-are-in-the-unmapped-data-part-of-pe
$(SHELLCODE): $(OBJDIR) $(SHELLOBJ)
	$(CC) $(CFLAGS) $(SHELLOBJ) -o $(SHELLCODE)
	@strip $(SHELLCODE) # We have a lot of unmapped data at the end, pretty odd...

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
