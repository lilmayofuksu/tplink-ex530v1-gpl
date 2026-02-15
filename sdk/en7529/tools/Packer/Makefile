
NAME		= packer

SRC		= src/packer.c		\
		  src/cypher_code.c	\
		  src/write_file.c	\
		  src/map_elf.c



OBJ		= $(SRC:.c=.o)

INCLUDES	= ./includes

CC		= gcc

NASM		= nasm

RM		= rm -f

override CFLAGS	+= -g -Wall -Wextra -std=c99 -I $(INCLUDES)



$(NAME):	$(OBJ) 
		$(CC) $(CFLAGS) -o $(NAME) $(OBJ) 


all:		$(NAME)

clean:
		$(RM) $(OBJ) 
		$(RM) $(NAME)

fclean:		clean
		$(RM) $(NAME) exec

re:		fclean all

.PHONY:		all clean fclean re
