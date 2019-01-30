all: runsc runcs clean

runsc: system_call.c
	gcc -o runsc system_call.c 
runcs: context_switch.c
	gcc -o runcs context_switch.c -pthread
clean:
	find . -type f -executable -delete	
