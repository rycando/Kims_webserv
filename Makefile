NAME = webserv
CC = clang++

SRCS = ./srcs/Server.cpp ./srcs/Config.cpp ./srcs/Client.cpp ./srcs/Handler.cpp ./srcs/HandlerMethods.cpp ./srcs/utils.cpp main.cpp

OBJS = $(SRCS:.cpp=.o)

.cpp.o : 
	$(CC) $^ -o $@ -g

$(NAME) : $(OBJS)
	$(CC) -I./includes/ $^ -o $@ -g

all : $(NAME)

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all
