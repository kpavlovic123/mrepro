INPUT_FILES := bot.c UDP_server.c
CLEAN_FILES := Makefile CandC.py $(INPUT_FILES)

build: $(INPUT_FILES:%.c=%)

%: %.c

clean: 
	find . $(CLEAN_FILES:%=! -name %)  -type f -delete
