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
CC = clang++ -g # -Wall -Wextra -Werror
RM = rm -rf

# DIRECTORIES

DHEADERS = ./inc/
DSRCS	= ./srcs/
DOBJS	= ./comp/

HEADERS = ./inc/*
# SOURCES

<<<<<<< HEAD
SRCS = main.cpp Config.cpp Server.cpp Client.cpp Request.cpp Response.cpp Utils.cpp Methods.cpp CGI.cpp
=======
SRCS = main.cpp Config.cpp Server.cpp Client.cpp Request.cpp Response.cpp Utils.cpp methods.cpp
>>>>>>> alex
OBJS = $(SRCS:%.cpp=$(DOBJS)%.o)

# COMPILATION

all: $(NAME)

run: all
	printf "\n$(_GREEN) 📡 Webserv Ready 📡$(_END)\n\n"
	./$(NAME)


$(NAME): $(OBJS) $(HEADERS)
	$(CC) -I $(DHEADERS) $(OBJS) -o $(NAME)

$(OBJS): | $(DOBJS) # https://www.gnu.org/software/make/manual/make.html

$(DOBJS):
	mkdir $(DOBJS)

$(DOBJS)%.o: $(DSRCS)%.cpp
	$(CC) -I $(DHEADERS) -c $< -o $@

# CLEAR

clean:
	$(RM) $(DOBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean re fclean

# TODO : Header Recompilation
# -include *.d
# https://stackoverflow.com/questions/35676039/makefile-rebuild-with-modified-header-files