TARGET := tcpserver.c tcpklijent.c
FILES := Makefile $(TARGET)

build: $(TARGET:%.c=%)
	
%: %.c
	gcc $< -o $@

clean: 
	find . $(FILES:%=! -name %) -type f -delete
