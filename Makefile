NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++17 -I$(INCLUDE_DIR) -g  -fsanitize=address
VPATH = ./config:./server:./utils
OBJECT_DIR = obj
INCLUDE_DIR = ./
SOURCES = main.cpp TcpSocket.cpp HttpServer.cpp Client.cpp Config.cpp Lexer.cpp ContextNode.cpp TokenStream.cpp HttpParser.cpp HttpResponse.cpp HttpUpload.cpp helper.cpp CGI.cpp
OBJS:= $(addprefix $(OBJECT_DIR)/, $(SOURCES:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

$(OBJS): $(OBJECT_DIR)/%.o: %.cpp $(HEADERS) | $(OBJECT_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECT_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
