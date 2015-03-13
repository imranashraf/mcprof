# mcprof #

`mcprof` is a memory and communication profiler. It traces memory reads/writes and reports memory accesses by various functions in the application as well as the data-communication between functions. The information is obtained by performing dynamic binary instrumentation by utilizing [Intel Pin](www.pintool.org) framework. An example application and the `mcprof` generated communication graph is shown in the figure below. This README file briefly explains the process of setting up `mcprof` and start generating such graphs for C/C++ applications.

Input Code  | Output Graph
------------- | -------------
![code](https://bitbucket.org/imranashraf/mcprof/raw/master/doc/example/exampleCode.png) | ![graph](https://bitbucket.org/imranashraf/mcprof/raw/master/doc/example/exampleGraph.png)

## Licensing

Copyright (c) 2014-2015 TU Delft, The Netherlands.
All rights reserved.

MCPROF is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

MCPROF is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with MCPROF.  If not, see <http://www.gnu.org/licenses/>.


## Reference

**To cite MCPROF Tool:**
```
"MCProf: Open-source Memory and Communication Profiler"
Imran Ashraf
URL: https://bitbucket.org/imranashraf/mcprof
```

**To cite MCPROF Design OR MCPROF Shadow Memory Technique:**
```
"Intra-Application Data-Communication Characterization"
Imran Ashraf, Vlad-Mihai Sima and Koen Bertels
Technical report, Delft University of Technology, 2015
```


## Download

`mcprof` can be downloaded [from](https://bitbucket.org/imranashraf/mcprof/downloads).

## Required Packages

In order to setup and use `mcprof` the following two packages are required:

* [Intel Pin DBI framework](www.pintool.org) Revision 62732 or higher
* g++ compiler
* graphviz Dot utility for converting the generated communication graphs from dot to pdf, png etc

## Setup

`mcprof` uses Makefile to compile the sources. In order to compile `mcprof` from sources on 32-bit / 64-bit Linux, the following steps can be performed.

* Download Pin and copy and extract it to the directory where you want to keep Pin.
* Define a variable `PIN_ROOT` by running the following commands:
        export PIN_ROOT=/<path to pin>
* You can also add this line, for instance, to your `.bashrc` in case you are using `bash` to export the variable automatically on opening a terminal.
* Download `mcprof` and copy and extract it to the directory where you want to compile it.

* Go the `mcprof` directory and run the following command to compile:
        make

If every thing goes fine, you will see a directory `obj-intel64` (or `obj-ia32` depending upon your architecture).

## Usage

Various test examples are available in `tests` directory inside `mcprof`. These tests are also compiled to binaries with the compilation process and are available as executables in obj-intel64/obj-ia32 directory. In order to run `mcprof`
for one of these tests (e.g. vecOps.cpp) you can run (from the `mcprof directory`) the following command:
    make vecOps.test

This will create `communication.dot` file which can be converted to pdf file by the following command:

    dot -Tpdf communication.dot -o communication.pdf

In order to use `mcprof` to analyze your application, you should compile your application with  `-g -O1 -fno-inline` compiler flags and link it to make the binary. For instance if the name of your application binary is`myApp` then you can use `mcprof` as:

    /<path to pin dir>/pin -t <path to mcprof dir>/obj-intel64/mcprof.so -- <path to your binary>/myApp

As above, this will generate `communication.dot` file which can be converted to pdf (or some other formats of your interest) for visualization.


## Detailed Documentation

Detailed documentation of `mcprof` with input options and outputs generated can be found in the [manual.pdf](https://bitbucket.org/imranashraf/mcprof/raw/master/doc/manual.pdf) in the  [doc](https://bitbucket.org/imranashraf/mcprof/raw/master/doc) directory.


## Who do I talk to?

In case you are interested in contributing to `mcprof`, or you have suggestions for improvements, or you want to report a bug, contact:

* Imran Ashraf [I.Ashraf@TUDelft.nl](I.Ashraf@TUDelft.nl)