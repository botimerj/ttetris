# ========
# Authors:
# ========
# Jacob C. Botimer (jake)

# Compiler
CXX		:= g++
INC		:= -Iinc/
FLAGS	:= -O2

# Link libraries
LINK	:= -lpthread

# Source files
SRC 	:= $(wildcard src/*.cpp)
SRC 	+= $(wildcard *.cpp)

# Object files
OBJ		:= $(subst .cpp,.o,$(SRC))
OBJ 	:= $(addprefix obj/,$(notdir $(OBJ)))

all: ttetris 

ttetris: $(OBJ)
	$(CXX) $(FLAGS) $(INC) $(OBJ) -o ttetris $(LINK)
	
.SECONDARY: $(OBJS)
obj/%.o: src/%.cpp inc/%.h
	@echo "CXX $(notdir $<)"
	@$(CXX) $(FLAGS) $(INC) -c $< -o $@

obj/%.o: %.cpp
	@echo "CXX $(notdir $<)"
	@$(CXX) $(FLAGS) $(INC) -c $< -o $@

clean:
	rm -f obj/* ttetris 

.PHONY: clean
