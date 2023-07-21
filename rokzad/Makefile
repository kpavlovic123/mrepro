TARGET := knock_server.c
FILES := Makefile $(TARGET)

build: $(TARGET:%.c=%)
	
%: %.c
	gcc $< -o $@

clean: 
	find . $(FILES:%=! -name %) -type f -delete