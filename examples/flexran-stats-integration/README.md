In this directory is a sample project that shows how to link a ``standalone''
agent (e.g., within a base station) to the FlexRIC agent once it has been
installed system wide from the DEB package.

First, create the deb package and install it as described in the README.md at
the repository root.

Then, change into this directory and build it as a separate cmake project:
```
$ cd examples/flexran-stats-integration
$ mkdir build
$ cd build
$ cmake ..
$ make
```
This builds three different sample agents that each use MAC, PDCP, or RRC RAN
functions and connect to a FlexRAN-like controller specialization.

Note:
* the controller should use the same E2AP serialization as is linked into the
  FlexRIC agent library
* the CMakeLists.txt hardcodes the path to the FlatCC library, which you might
  need to override
* the example uses the FindFlatCC.cmake of the FlexRIC project to find FlatCC.
