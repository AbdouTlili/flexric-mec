# FlexRIC: The O-RAN Alliance compliant E2-Agent and NearRT-RIC 

This repository contains O-RAN Alliance (https://www.o-ran.org/) compliant E2-Agent emulators, a NearRT-RIC, xApps written in C/C++ and Python, as well as patches to 5G/4G OpenAirInterface and 4G srsRAN. Additionally, it stores the data received in the xApp in a sqlite3 database for offline processing applications (e.g., ML/AI). 


## General instructions

To build this repository, you need a *recent* CMake. On Ubuntu, you might want
to use [this PPA](https://apt.kitware.com/) to install an up-to-date version.

Download the required dependencies. 

```bash
sudo apt install libsctp-dev python3.8 cmake-curses-gui libpcre2-dev 
```

We use SWIG (i.e., > 4.0) as an interface generator to enable the multi-language feature (i.e., C/C++ and Python) for the xApps.

Please, check your SWIG version (i.e, $ swig -version) and install it from scratch if necessary as described here: https://swig.org/svn.html 
or
```bash
$ git clone https://github.com/swig/swig.git
$ cd swig
$ ./autogen.sh
$ ./configure --prefix=/usr/
$ make -j $(nproc)
$ make install
```

You can now clone the FlexRIC project and build it. 

```bash
$ git clone https://gitlab.eurecom.fr/mosaic5g/flexric.git 
$ cd flexric && mkdir build && cd build && cmake .. && make -j $(nproc)
```

You also need to install the Service Models (SM) in your computer.

```bash
sudo make install
```

By default they will be installed in the path /usr/local/flexric

Check that everything is working correctly by running. 

```bash
$ ./build/test/test_near_ric # located in the /path/to/flexric/build
```

Before starting the nearRT-RIC, check that the IP address where your nearRT-RIC will be listening is the desired one at /usr/local/flexric/flexric.conf. You can now start the nearRT-RIC. 

```bash
$ ./build/examples/ric/nearRT-RIC
```

You can also start multiple E2 Agent emulators that will try to connect to the IP address specified at /usr/local/flexric/flexric.conf.

```bash
$ ./build/examples/emulator/agent/agent_1
$ ./build/examples/emulator/agent/agent_2
$ ./build/examples/emulator/agent/agent_3
$ ./build/examples/emulator/agent/agent_4
$ ./build/examples/emulator/agent/agent_5
```

Next, you can fetch some statistics from the E2 Agents using python. 

```bash
$ cd build/example/xApp/python3/
$ python3 xapp_mac_rlc_pdcp_moni.py
```

While in other window you can start a second xApp developed in c

```bash
$ cd build/example/xApp/c/monitor/
$ ./xapp_mac_rlc_pdcp_moni
```

You can now start wireshark and see how E2AP messages are flowing.

The latency that you observe in your monitor xApp is the latency from the E2 Agent to the nearRT-RIC and xApp. In modern computers the latency should be around 200 microseconds or 50x faster than the O-RAN specified maximum nearRT-RIC latency of 10 ms.

At this point, FlexRIC is working correctly in your computer and you have already tested the multi-agent, multi-xApp and multi-language capabilities. 

Additionally, all the data received in the xApp has also been written to /tmp/xapp_db in case that offline data processing is wanted (e.g., Machine Learning/Artificial Intelligence applications).

Please, check the example folder for other working xApp use cases.

## Integration with OpenAirInterface 4G/5G

```bash
$ git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git oai
$ cd oai/
$ git checkout nr-mac-rlc-pdcp-stats
$ git am path/to/flexric/multiRAT/oai/oai.patch --whitespace=nowarn
$ source oaienv
$ cd cmake_targets
$ ./build_oai -I -w USRP -i  #For OAI first time installation
$ ./build_oai --eNB --gNB -c -C -w USRP --ninja
```

Prepare you favourite hot/cold beverage as the compilation of OAI may take 10 minutes. 
Example configuration files using a B210 USRP are provided to facilitate the integration.

You can now run the 5G or 4G OAI gNodeB/eNodeB
```bash
# eNB
$ cd oai/cmake_targets/ran_build/build
$ sudo ./lte-softmodem -O path/to/flexric/multiRAT/oai/enb.band7.tm1.25PRB.usrpb210.replay.conf

# gNB
$ cd oai/cmake_targets/ran_build/build
$ sudo ./nr-softmodem -O path/to/flexric/multiRAT/oai/gnb.sa.band78.fr1.106PRB.usrpb210.conf --sa -E --continuous-tx
```

In this release MAC, RLC, PDCP SMs are operative for 5G/4G and slicing for 4G 

## Integration with srsRAN 4G

Install srsRAN from source <https://docs.srsran.com/en/latest/general/source/1_installation.html#installation-from-source> and follow their instructions.


```bash
cd path/to/srsran
git checkout release_21_10
git am path/to/flexric/multiRAT/srsran/srsenb.patch --whitespace=nowarn
mkdir build && cmake .. && make -j8 
cd srsenb/src/
sudo ./srsenb
```

In this release MAC, RLC, PDCP and partially Slicing are supported SMs.


## Demo July 2022

If you want to see the full power of FlexRIC using their multi-RAT, multi-vendor, multi-language, multi-agent and multi-xApp in action follow the next steps.


XXXXXX Here Chieh


## Flatbuffers 
We also provide a flatbuffers encoding/decoding scheme as alternative to ASN.1. In case that you want to use it,
follow the instructions at https://github.com/dvidelabs/flatcc and provide the path for the lib and include when
selecting it at ccmake .. from the build directory 

## Research Paper
If you want to know more about FlexRIC and its architecture, you can find more details at:

Robert Schmidt, Mikel Irazabal, and Navid Nikaein. 2021. FlexRIC: an SDK for next-generation SD-RANs. In Proceedings of the 17th International Conference on emerging Networking EXperiments and Technologies (CoNEXT '21). Association for Computing Machinery, New York, NY, USA, 411–425. DOI:https://doi.org/10.1145/3485983.3494870


