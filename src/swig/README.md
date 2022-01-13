# SWIG python wrapper for the RIC

## General instructions
This project has swig and python dependencies.
In a Ubuntu machine:
```bash
$ sudo apt-get update 
$ sudo apt-get install swig
$ sudo apt-get install python3.8 
```
It also needs that FlexRIC has been built as a shared object.

```bash
$ mkdir build && cd build && cmake .. 
$ ccmake ..
$ make -j$(nproc) 
```
You can now directly access FlexRIC as a Python3 module

```bash
$ python3  
$ import flexric as ric 
$ ric.init()
```



