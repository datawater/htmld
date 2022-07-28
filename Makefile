CFLAGS = -Wall -Wextra -std=c11
OUTPUT= out/main

main:
	rm out/ -fdr
	mkdir out
	make htmld

release:
	rm out/ -fdr
	mkdir out
	make release-htmld

htmld: src/main.c
	$(CC) $(CFLAGS) -o out/main -O1 -ggdb src/main.c

release-htmld: src/main.c
	$(CC) $(CFLAGS) -o out/main -O2 -s src/main.c

test: release
	./out/main -input ./test/index.html -output ./out/out.html
	@echo "Checkout ./out/out.html in any browser!"

todo:
	@grep "TODO: " -rn . --exclude="flag.h" --exclude="Makefile" --exclude="TODO.md" --color=always