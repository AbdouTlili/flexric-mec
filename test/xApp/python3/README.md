Python3 xApp example

Instructions to run it correctly:

1- cd to path/to/flexric/build
2- Type: ccmake ..
3- Select DYNAMIC build instead of STATIC and save the changes pressing cg
4- Type: make
5- cd to path/to/flexric/src/xApp/swig/
6- Type: mkdir build && cd build && cmake .. && make -j
7- You need 2 running agents and one RIC for this script, so cd path/to/flexric/build/test/agent
8- Type ./agent\_42 
9- Open new window and type: ./agent\_43
10- cd path/to/flexric/build/test/ric 
11- Type: ./near\_ric\_iapp 
12- cd path/to/flexric/test/xApp/python3/
13- Type: python3 test.py 

Enjoy!

