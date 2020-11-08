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
CC = clang++ -Wall -Wextra -Werror # -g3 -fsanitize=address
RM = rm -rf

# DIRECTORIES

DHEADERS = ./inc/
DSRCS	= ./srcs/
DOBJS	= ./comp/


HEADERS = ./inc/*
# SOURCES

SRCS = main.cpp Parser.cpp Config.cpp Server.cpp Client.cpp Request.cpp Response.cpp Utils.cpp Methods.cpp CGI.cpp
OBJS = $(SRCS:%.cpp=$(DOBJS)%.o)

ifeq ($(SILENTLOGS), 1)
	LOGS=1
else
	LOGS=0
endif

# COMPILATION

# make run SILENTLOGS=1
# make test SILENTLOGS=1
# make re SILENTLOGS=1

all: $(NAME)

run: all
	@printf "\033c"
	@./$(NAME) conf/default.conf

test: all
	@printf "\033c"
	@./$(NAME) conf/tester.conf

$(NAME): $(OBJS) $(HEADERS)
	$(CC) -I $(DHEADERS) $(OBJS) -o $(NAME) 

$(OBJS): | $(DOBJS) # https://www.gnu.org/software/make/manual/make.html

$(DOBJS):
	mkdir $(DOBJS)

$(DOBJS)%.o: $(DSRCS)%.cpp
	$(CC) -I $(DHEADERS) -c $< -o $@ -D SILENTLOGS=$(LOGS)

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