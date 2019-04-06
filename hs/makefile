cube: wizard.o cube.o
	cc -g cube.o wizard.o -lreadline -lhistory -lncurses -lpthread -o cube

clean: 
	rm cube wizard.o cube.o

cube.o: cube.c cube.h
	cc -c cube.c

wizard.o: wizard.c wizard.h
	cc -c wizard.c
