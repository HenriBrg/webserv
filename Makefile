# VARIABLES

NAME = webserv
CPP = clang++ # -Wall -Wextra -Werror
SRCS = srcs/main.cpp srcs/utils.cpp
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
	./$(NAME) x


# CLEAN

clean:
	rm $(OBJS)

fclean: clean
	rm $(NAME)

re: fclean all

.PHONY: re fclean all