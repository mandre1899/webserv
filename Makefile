NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++17 -I$(INCLUDE_DIR) -g
SOURCE_DIR = src
OBJECT_DIR = obj
INCLUDE_DIR = inc
SOURCES = main.cpp TcpSocket.cpp HttpServer.cpp Client.cpp ConfigParser.cpp webserv.cpp
OBJS:= $(addprefix $(OBJECT_DIR)/, $(SOURCES:.cpp=.o))

#   -fsanitize=address

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

$(OBJS): $(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(HEADERS) | $(OBJECT_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECT_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re