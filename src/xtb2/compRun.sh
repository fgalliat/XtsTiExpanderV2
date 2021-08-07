rm a.out 2>/dev/null

g++ -c xtbase.cpp && \
g++ -c main.cpp && \


g++ xtbase.o main.o && \

rm *.o && \

./a.out $*
# ls
