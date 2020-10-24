build: chess.cpp 
	g++ chess.cpp -o chess
run: chess
	./chess
clean: chess
	rm -rf chess
