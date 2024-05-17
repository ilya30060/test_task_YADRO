all: build run
	

build:
	g++ task.cpp -o task
run:
	task.exe test_file.txt