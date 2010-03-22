.PHONY: macports, clean

waveformgen:
	gcc  -Wall -O3 -std=c99 -o waveformgen -lm -lsndfile -lgd -lpng waveformgen.c

macports:
	gcc  -Wall -O3 -std=c99 -I/opt/local/include -L/opt/local/lib -o waveformgen -lm -lsndfile -lgd -lpng waveformgen.c
		
clean: 
	rm waveformgen