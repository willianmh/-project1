cmake_minimum_required(VERSION 3.8)
project(Trab1)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c montador.h token.h processarEntrada.c token.c emitirMapaDeMemoria.c)
add_executable(Trab1 ${SOURCE_FILES})