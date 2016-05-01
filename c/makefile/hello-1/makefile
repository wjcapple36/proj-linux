#hello:main.o print_hello.o
#	cc -o hello main.o print_hello.o
#main.o:main.c
#	cc -c main.c
#print_hello.o:print_hello.c print_hello.h
#	cc -c print_hello.c
#clean :
#	rm main.o print_hello.o hello
OBJ_DIR=./src
VPATH=src:include
#vpath %.h include
sources=$(wildcard $(OBJ_DIR)/*.c)
objects=$(patsubst %.c,%.o,$(sources))
#echo $(sources)
hello : $(objects)
	@echo "$$ ^ >" $^
	@echo "$$ @ >" $@
	cc  $^ -o $@
clean:
	-rm hello $(OBJ_DIR)/*.o
	-rm $(OBJ_DIR)/*.d*
display:
	@echo "VPATH>>" $(VPATH)
	@echo "sources>>"$(sources)
	@echo "objects>>"$(objects)
#sources=main.c print_hello.c
include $(sources:.c=.d)
%.d: %.c
	set -e; rm -f $@; \
$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
sed 's,\($(notdir $*)\)\.o[:]*,$(dir $*)\1.o $@:,g' < $@.$$$$ > $@; \
#rm -f $@.$$$$
