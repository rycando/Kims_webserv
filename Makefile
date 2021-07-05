NAME = webserv
CC = clang++
FLAGS = -Wall -Wextra -Werror -I./includes

SRCS = main.cpp ./srcs/Config.cpp ./srcs/Server.cpp ./srcs/Client.cpp ./srcs/Handler.cpp ./srcs/HandlerMethods.cpp ./srcs/utils.cpp

OBJS = $(SRCS:.cpp=.o)

$(NAME) : $(OBJS)
	$(CC) $(FLAGS) $^ -o $@ -g

all : $(NAME)

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all
