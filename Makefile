simfs_test: simfs_test.c simfs.a
	gcc -DCTEST_ENABLE -Wall -Wextra -o $@ $^

block.o: block.c image.h image.c
	gcc -c -Wall -Wextra -o $@ $<

image.o: image.c
	gcc -c -Wall -Wextra -o $@ $<

free.o: free.c
	gcc -c -Wall -Wextra -o $@ $<

inode.o: inode.c block.h block.c free.h free.c
	gcc -c -Wall -Wextra -o $@ $<

mkfs.o: mkfs.c image.h image.c block.h block.c
	gcc -c -Wall -Wextra -o $@ $<

simfs.a: block.o image.o free.o inode.o mkfs.o 
	ar rcs $@ $^

.PHONY: test

test: simfs_test
	./simfs_test
	rm -f simfs_test

.PHONY: clean test

clean:
	rm -f *.o test_image