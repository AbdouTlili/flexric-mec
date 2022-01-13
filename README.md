# FlexRIC: The O-RAN Alliance compliant E2-Agent and Near-RIC 

This repository contains an O-RAN Alliance (https://www.o-ran.org/) compliant E2-Agent as well as a Near-RIC 

## General instructions

To build this repository, you need a *recent* CMake. On Ubuntu, you might want
to use [this PPA](https://apt.kitware.com/) to install an up-to-date version.


Building instructions: 
* E2AP messages are protected by the O-RAN license. To acquire them, visit https://www.o-ran.org/specifications and sign the O-RAN ADOPTER LICENSE AGREEMENT. 
The definitions of the E2AP messages can be found at the O-RAN.WG3.E2AP-v01.01.pdf.

Download the required dependencies 

```bash
sudo apt install asn1c libsctp-dev poppler-utils python3.8
```

Today, the xApp communication is based on the Next Generation of Nanomessages https://github.com/nanomsg/nng. Please go there and install them into your computer.
You can now clone the FlexRIC project 


```bash
$ git clone https://gitlab.eurecom.fr/mosaic5g/flexric.git 
```

To convert the ASN.1 module files into C structs, we provide a python3 script 

```bash
$ cd src/lib/ap/ie/asn
$ python3 gen_asn.py path/to/O-RAN.WG3.E2AP-v01.01.pdf 
```
At this point you can go back and compile the project.


```bash
$ cd ../../../../../ && mkdir build && cd build && cmake ..
$ ccmake .. # to see all building options 
```
By default, CMake will trigger a DEBUG build.
You can now make your project, install the Service Models, and run a E2-Agent Near-RIC test that will generate MAC, RLC
and PDCP data and store it in a log.txt file

```bash
$ make -j 
$ sudo make install 
$ ./test/test_near_ric
```


## Integration with OpenAirInterface

To facilitate the E2-Agent integration with OAI we provide a patch at the directory oai/

```bash
$ git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git oai
$ cd oai && git checkout b04731d7565cd91b538eb7cc80f874b4730d54ad 
$ git am /path/to/flexric/oai/0001-FlexRIC-s-E2-Agent-integration-in-OAI-5G.patch  
$ git am /path/to/flexric/oai/0002-Last-E2-Agent.patch

```
Go to executables/nr-softmodem.c and change the Near-RIC IP address 
You can now compile OAI

```bash
$ cd cmake_targets && ./build_oai -c -C -w USRP --gNB
```

Prepare you favourite hot beverage as the compilation of OAI may take 10 minutes. 
The configuration file used during the Hackfest 2021, where a B210 USRP was utilized, is also provided to facilitate the integration.
Following the commands to run the 5G SA base station.

```bash
$ cd ../targets/bin/
$ sudo ./nr-softmodem.Rel15 -O path/to/flexric/oai/gnb.band78.tm1.fr1.106PRB.usrpb210.conf -E  --sa --usrp-tx-thread-config 1 
```

You can now start a Near-RIC and see the communication between the E2-Agent and the Near-RIC (e.g., using Wireshark).   
If you want to use the xApps, start a Near-RIC, report the services for the SMs (i.e., using the call report_service_near_ric_api) and start an infinite loop.
You can then connect to the Near-RIC through the xApps that were provided during the Hackfest 2021.

## Flatbuffers 
We also provide a flatbuffers encoding/decoding scheme as alternative to ASN.1. In case that you want to use it,
follow the instructions at https://github.com/dvidelabs/flatcc and provide the path for the lib and include when
selecting it at ccmake .. from the build directory 

## Research Paper
If you want to know more about FlexRIC and its architecture, you can find more details at:

Robert Schmidt, Mikel Irazabal, and Navid Nikaein. 2021. FlexRIC: an SDK for next-generation SD-RANs. In Proceedings of the 17th International Conference on emerging Networking EXperiments and Technologies (CoNEXT '21). Association for Computing Machinery, New York, NY, USA, 411–425. DOI:https://doi.org/10.1145/3485983.3494870


