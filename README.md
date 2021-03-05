# FlexRIC: Server and Agent

This repository contains both the server and agent of the FlexRIC SD-RAN
Controller.

## General instructions

To build this repository, you need a *recent* CMake. On Ubuntu, you might want
to use [this PPA](https://apt.kitware.com/) to install an up-to-date version.

Other dependencies:
* Flatbuffers: see below for build instructions.

To build the different executables, follow the instructions below. You need to
execute once before:
```bash
$ mkdir build && cd build && cmake .. # see Flatbuffers instructions below
$ ccmake .. # to see all cmake variables and set options
```

By default, cmake will trigger a DEBUG build, which includes AddressSanitizer
and gcov.

## Use Flatbuffers for E2AP

FlexRIC uses the [FlatCC Flatbuffers in C for
C](https://github.com/dvidelabs/flatcc/) library for flatbuffers (i.e., not the
official Google library, which is not available in C). You have to install
flatcc from source (which is quick). Instructions (change paths if you don't
want to clone/install into your home):
```bash
$ cd
$ git clone https://github.com/dvidelabs/flatcc.git
$ cd flatcc
$ ./scripts/build.sh
```
This compiles flatcc and puts header files in `~/flatcc/include/flatcc/` and
the library into `~/flatcc/lib`. Then, configure FlexRIC to use flatbuffers and
using the correct paths to find flatbuffers:
```
$ cd <flexric-build-dir>
$ cmake .. -DFlatCC_HINT_INCLUDE_DIR=~/flatcc/include/ -DFlatCC_LIBRARY_DIR=~/flatcc/lib/
```
Then, proceed building as normal. This will use Flatbuffers for E2AP, and no
ASN.1 (you don't need it). Note that currently we still build with the E2SM-HW,
E2SM-NI, and E2SM-KPM libraries, which build the SMs using ASN.1.

## Server

A simple standalone server is in `src/server/flexric_rtc.c`. After the E2 Setup
Request/Response, a hardcoded subscription request is sent to the agent. Build
and run:
```bash
$ make -j$(nproc)
$ ./src/flexric/flexric_rtc
```

## Agent

The standalone agent *library* can be built with:
```bash
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j$(nproc)
$ cpack
$ sudo apt-get install <file> # replace <file> with .deb file from cpack generation
```

## End-to-end testing

An end-to-end test of FlexRIC Agent and Server can be built and run with
```
$ make e2ap_test -j$(nproc)
$ ./test/e2ap_test
```

## Run tests

To run the tests, build as normal, then run
```
$ make test
```

## Build FlexRAN controller specialization with REST northbound

FlexRAN is always built, except for the REST northbound, which has a dependency
on [Pistache](https://github.com/pistacheio/pistache). To also build the REST
northbound, first install Pistache:
```
$ sudo add-apt-repository ppa:pistache+team/unstable
$ sudo apt update
$ sudo apt install libpistache-dev
```
Then, enable the REST northbound with either command:
```
$ cmake .. -DREST_NORTHBOUND=ON
$ ccmake .. # enable manually in the menu
```

## Build ASN.1 E2AP

In order to build FlexRIC with ASN.1 E2AP encoding, generate the E2AP ASN.1
definitions in the directory `src/msg/e2/ap`, as described in the file
`src/msg/e2/README.md`. Then, compile like this:
```
$ cd build
$ cmake -DE2AP_ENCODING=ASN ..
$ make -j$(nproc)
```

## Build SM-HW using ASN.1

In order to build HW iApp using ASN.1 (`src/sm/asn1_hw`), do:
1. Clone the HW xApp and check out commit edcbaf8c8ab1bfdf605df32ee20bb6f4f4137a7d:
```
$ git clone "https://gerrit.o-ran-sc.org/r/ric-app/hw"
$ cd hw
$ git checkout edcbaf8c8ab1bfdf605df32ee20bb6f4f4137a7d
```
2. Copy the HW ASN.1 definitions to `src/msg/e2/sm-hw`
```
$ cp -r asn1c_defs/*.{c,h} ~/flexric/src/msg/e2/sm-hw/
```
3. Enable building SM-HW in cmake and compile:
```
$ cmake -DSM_HW=ON ..
$ make -j$(nproc)
```

## OAI Integration

The FlexRIC integration is based on OAI, tag 2021.w20. To integrate it, proceed
as follows:
1. Build the FlexRIC agent library *as Release build* as shown in section Agent
   above, then install it:
```
$ sudo apt-get install flexric-agent_0.2.2_amd64.deb
```
2. Checkout OAI tag `2021.w20`, and apply the integration patch.
```
$ cd <oai-repo>
$ git checkout 2021.w20 # checkout w20 in OAI
$ git apply ~/flexric/oai/0001-Complete-FlexRIC-integration.patch
```
3. Modify the target IP address, if necessary, in
   `openair2/ENB_APP/flexric_agent.c` or
   `openair2/GNB_APP/flexric_agent_nsa.c` (probably to `127.0.0.1`).
3. Compile OAI as normal. You should see the following line appear:
```
-- Found FlatCC: /home/<user>/flatcc/lib/libflatccrt.a
-- Found FlexricAgent library
```
4. Start a FlexRIC specialization, e.g., FlexRAN:
```
$ ./build/src/controller/flexran/flexran_rtc
```
   Then start OAI, you should see a connection happening. In FlexRIC, you
   should be able to retrieve statistics if you compiled with the north-bound:
```
$ curl localhost:9999/stats | jq
```
