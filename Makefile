# COLORS

_RED		=	\e[31m
_GREEN		=	\e[32m
_YELLOW		=	\e[33m
_BLUE		=	\e[34m
_END		=	\e[0m

# ifndef VERBOSE
# .SILENT:
# endif

# VARIABLES

NAME = webserv
CC = clang++ # -Wall -Wextra -Werror
RM = rm -rf

# DIRECTORIES

DHEADERS = ./includes/
DSRCS	= ./srcs/
DOBJS	= ./comp/

# SOURCES

SRCS = main.cpp Config.cpp Server.cpp Client.cpp Request.cpp Response.cpp
OBJS = $(SRCS:%.cpp=$(DOBJS)%.o)

# COMPILATION

all: $(NAME)

run: all
	./$(NAME)

$(NAME): $(OBJS)
	$(CC) -I $(DHEADERS) $(OBJS) -o $(NAME)

$(OBJS): $(DOBJS)

$(DOBJS)%.o: $(DSRCS)%.cpp
	$(CC)  -I $(DHEADERS) -c $< -o $@

$(DOBJS):
	mkdir $(DOBJS)

clean:
	$(RM) $(DOBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean re fclean