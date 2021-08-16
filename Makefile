smallsh:
	gcc -std=c11 -Wall -Werror -g3 -O0 smallsh.c -o smallsh

test:
	./p3testscript 2>&1
	# ./p3testscript 2>&1 | more
	# ./p3testscript > mytestresults 2>&1

clean:
	rm -f ./smallsh
	rm -rf __MACOSX
	rm -f .DS_Store
