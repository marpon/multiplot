

NAME	=	test_multiplot

SRC		=	$(wildcard *.cpp)

FLAGS	=	-Wall -Wextra -Werror -Os -std=c++11

LFLAGS	=	-s -static -lgdi32 -lopengl32

RM		=	rm -rf


OBJ_F	=	$(SRC:.cpp=.o)

DEP_F	= 	$(OBJ_F:.o=.d)

NAME_EXE = 	$(NAME).exe
	
CC		=	g++

all:  $(NAME_EXE) 
	
$(NAME_EXE): $(OBJ_F)
	@echo.
	@echo.
	@echo Linking ...  
	$(CC)  -o $@ $^ $(LFLAGS)
	@echo.
	@echo   Executable created : $@ 
	
	
$(OBJ_F): %.o: %.cpp Makefile
	@echo.
	@echo.
	@echo Compiling ...  
	$(CC) -c -MMD -MP $(FLAGS) -o $@ $<
	
-include $(DEP_F)


clean:
	$(RM) $(OBJ_F)
	$(RM) $(DEP_F)	

fclean: clean
	$(RM) $(NAME_EXE)

re: fclean all

.PHONY : re fclean clean all 