
all: filcio.so test 

filcio.so: filcio.c
	gcc -pthread -g -shared -fPIC filcio.c -o filcio.so -ldl

test: test.c
	gcc -g test.c -o test

opt: 
	gcc -O3 -pthread -g -shared -fPIC filcio.c -o filcio.so -ldl
	gcc -O3 -g test.c -o test

run: filcio.so test
	export AGG_BUFSIZE=32000000
	LD_PRELOAD=./filcio.so ./test

clean:
	-@rm test filcio.so perf_analysis/junk/* 2>/dev/null || true

