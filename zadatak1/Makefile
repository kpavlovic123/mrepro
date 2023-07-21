PROJECT = proba
SOURCES = proba.c
HEADERS = proba.h

OBJECTS = $(SOURCES:%.c=%.o)

PROJECT: $(OBJECTS)
	gcc -o $(PROJECT) $(OBJECTS)
%.o: %.c %.h
	gcc -c $(@:%.o=%.c)
clean:
	rm $(PROJECT) $(OBJECTS)
	
