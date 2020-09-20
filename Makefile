# VARIABLES

NAME = webserv
CPP = clang++ # -Wall -Wextra -Werror
SRCS = srcs/main.cpp srcs/Config.cpp srcs/Server.cpp srcs/Client.cpp srcs/Request.cpp srcs/Response.cpp
OBJS = $(SRCS:.cpp=.o)

# COMPILATION

ifndef VERBOSE
.SILENT:
endif

.cpp.o: 
	$(CPP) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(OBJS) -o $(NAME)

# TEST

x: $(NAME)
	./$(NAME)


# CLEAN

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: re fclean