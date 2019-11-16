all : compute package

compute : compute.c
	gcc compute.c -Wall -pthread -o compute -std=c99

package : package.c
	gcc package.c -Wall -o package -std=c99