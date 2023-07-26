#--------------FLAGS-----------------#

CC			=	cc
FLAGS		=	-Wall -Wextra -Werror

#--------------SOURCING--------------#

CLIENT_SRC	=	client.c
SERVER_SRC	=	server.c
INC			=	$(LIBFT_DIREC)

#--------------LIBRARY---------------#

LIBFT		=	./libft/libft.a
LIBFT_DIREC	=	./libft

#------------EXCECUTABLES------------#

NAME	=	minitalk
CLIENT	=	client
SERVER	=	server

all: $(SERVER) $(CLIENT)

$(NAME): all

$(SERVER): $(LIBFT) $(SERVER_SRC)
	@ $(CC) $(FLAGS) $(SERVER_SRC) $(LIBFT) -I $(INC) -o $(SERVER) && printf "server ✅\n"

$(CLIENT): $(LIBFT) $(CLIENT_SRC)
	@ $(CC) $(FLAGS) $(CLIENT_SRC) $(LIBFT) -I $(INC) -o $(CLIENT) && printf "client ✅\n"


$(LIBFT):
	@ $(MAKE) -C $(LIBFT_DIREC)

clean:
	@ rm -rf $(CLIENT) $(SERVER) && printf "object files 🗑️\n"

fclean: clean
	@ $(MAKE) fclean -C $(LIBFT_DIREC) && printf "executables 🗑️\n"

re: fclean all

.PHONY: all clean fclean re