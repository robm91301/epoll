This directory contains a small project done to experiment with epoll functionality.

There are the following files in the directory:

main.c  	the code for a small c server that receives input and reverse it
Makefile  	a make file that compiles the main.c code into a server executable
test.py		a test python script that makes sure that the server works correctly
README		this file
server		an executable compiled on Ubunutu 14.04

type make at the command line and start the server executable

in another window, run the python script to make sure that server is return a reversed bit array 

Improvements:

Enhance testing to check all elements of the array
Change server to handle abritarily large input

