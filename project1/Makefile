all: runsc runcs 
clean:
	find . -type f -executable -delete
runsc: system_call.c
	gcc -o runsc system_call.c 
runcs: context_switch.c
	gcc -o runcs context_switch.c 	
