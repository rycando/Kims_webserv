NAME = webserv
CC = clang++
FLAGS = -Wall -Wextra -Werror

SRCS = main.cpp ./srcs/Config.cpp ./srcs/Server.cpp ./srcs/Client.cpp ./srcs/Handler.cpp ./srcs/HandlerMethods.cpp ./srcs/utils.cpp

OBJS = $(SRCS:.cpp=.o)

.cpp.o : 
	$(CC) $(FLAGS) $^ -o $@ -g

$(NAME) : $(OBJS)
	$(CC) $(FLAGS) -I./includes/ $^ -o $@ -g

all : $(NAME)

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all
