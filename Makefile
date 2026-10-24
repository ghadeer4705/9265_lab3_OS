
CXX = g++
CXXFLAGS = -Wall -std=c++11

myshell: command.cc tokenizer.cc command.h tokenizer.h
	$(CXX) $(CXXFLAGS) command.cc tokenizer.cc -o myshell

clean:
	rm -f myshell
## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 
