TARGET = hello   
OBJ_PATH = .
PREFIX_BIN =  
  
CC = g++  
INCLUDES = -I include  
LIBS =  
CFLAGS =-Wall -Werror -O2 -g -pg 
LINKFLAGS =  
  
SRCDIR = ./ 
  
#C_SOURCES = $(wildcard *.c)  
C_SRCDIR = $(SRCDIR)  
C_SOURCES = $(foreach d,$(C_SRCDIR),$(wildcard $(d)/*.c) )  
C_OBJS = $(patsubst %.c, $(OBJ_PATH)/%.o, $(C_SOURCES))  
  
#CPP_SOURCES = $(wildcard *.cpp)  
CPP_SRCDIR = $(SRCDIR)  
CPP_SOURCES = $(foreach d,$(CPP_SRCDIR),$(wildcard $(d)/*.cpp) )  
CPP_OBJS = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(CPP_SOURCES))  
#%.o:%.c
#	@echo "$$< >>" $<
#	@echo "$$ @ >>"$@  
#	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
#$(CPP_OBJS):$(OBJ_PATH)/%.o:%.cpp
#	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@  
#init:
#	$(foreach d,$(SRCDIR), mkdir -p $(OBJ_PATH)/$(d);)

$(TARGET):$(C_OBJS) $(CPP_OBJS)
	$(CC)  $^ -o $(@)  $(LINKFLAGS) $(LIBS) $(CFLAGS) 
sources = $(C_SOURCES)
sinclude $(sources:.c=.d)
%.d:%.c
	set -e; rm -f $@; \
$(CC) -MM $(CPPFLAGS) $< > $@.$$$$;\
sed 's,\($(notdir $*)\)\.o[:]*,$(dir $*)\1.o $@:,g' < $@.$$$$ > $@;\
rm -f $@.$$$$
clean:  
	$(foreach d,$(SRCDIR),rm -f $(d)/*.o)
	$(foreach d,$(SRCDIR),rm -f $(d)/*.d)
	rm -f $(TARGET) 
 
test:  
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "C_OBJS: $(C_OBJS)"  
	@echo "CPP_SOURCES: $(CPP_SOURCES)"  
	@echo "CPP_OBJS: $(CPP_OBJS)" 
  
  
install: $(TARGET)  
	cp $(TARGET) $(PREFIX_BIN)  
  
uninstall:  
	rm -f $(PREFIX_BIN)/$(TARGET)  
  
rebuild: clean init compile 
 
