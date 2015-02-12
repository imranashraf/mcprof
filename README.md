# mcprof #

`mcprof` is a memory and communication profiler. It traces memory reads/writes and reports memory accesses by various functions in the application as well as the data-communication between functions. The information is obtained by performing dynamic binary instrumentation by utilizing [Intel Pin](www.pintool.org) framework. This README file briefly explains the process of setting up `mcprof` and using it.

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

Detailed documentation of `mcprof` with input options and outputs generated can be found in the `manual.pdf` in `doc` directory in `mcprof` *Comming soon*.


## Who do I talk to?

In case you are interested in contributing to `mcprof`, or you have suggestions for improvements, or you want to report a bug, contact:

* Imran Ashraf [I.Ashraf@TUDelft.nl](I.Ashraf@TUDelft.nl)