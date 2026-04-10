#
#	VARIABLES
#


# Name of the executable
NAME		=	IRC

# Compliler and flags to include on the exes
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -g

# Directory
SRC_DIR		=	src/

# Source files
SRC_FILES	=	\
				main.cpp \
				Server.cpp \
				ServerPoll.cpp \
				errorMsgs/ServerMsgs.cpp
				

SRC			=	$(addprefix $(SRC_DIR), $(SRC_FILES))

# Other Utils
RM			=	rm -f

#
#	RULES
#

all: $(NAME)

$(NAME):
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(NAME)
	@echo "Compiled exacutable $(NAME), execute it with './$(NAME)'"

clean:
	@$(RM) $(NAME)
	@echo "Cleaned exacutable $(NAME)"

fclean:
	@$(RM) $(NAME)
	@echo "Cleaned executable $(NAME)"

re: fclean all