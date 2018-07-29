all: proxy.c client.c
	mkdir cache
	gcc -o proxy proxy.c
	gcc -o client client.c
	./proxy -p 9530
clean:
	rm -rf cache/*
	rm -rf client proxy
	rmdir cache
