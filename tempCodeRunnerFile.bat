@echo off
g++ src/main.cpp -o server -I ./Crow/include -I ./asio/asio/include -I /mingw64/include -L /mingw64/lib -lpthread -lws2_32 -lmswsock
server.exe