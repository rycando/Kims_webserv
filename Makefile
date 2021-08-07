NAME = webserv
CC = clang++
FLAGS = -I./includes

SRCS = 	./srcs/Server.cpp \
		./srcs/Config.cpp \
		./srcs/Client.cpp \
		./srcs/Helper.cpp \
		./srcs/HelperStatusCode.cpp \
		./srcs/Handler.cpp \
		./srcs/HandlerMethods.cpp \
		./srcs/utils.cpp \
		./srcs/Communicate.cpp \
		main.cpp

# OBJS = $(SRCS:%.cpp=%.o)

.cpp.o : 
	$(CC) $(FLAGS) $^ -o $@

$(NAME) : 
	$(CC) $(FLAGS) $(SRCS) -o $(NAME) 

all : $(NAME)

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all
