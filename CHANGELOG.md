# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [ 2.1 ] - [ 2016-xx-xx ]
### Added
- dot graph filter script to apply node and edge threshold
- graph in .dat format as input for dotfilter script

### Changed
-

### Removed
- threshold as command line arg to mcprof in favor of filter script 

### Fixed
- 

## [ 2.1 ] - [ 2016-11-20 ]
### Added
- Engine 4: loop dependency support (under test)
- sparse matrix for loop dependence detection
- callgraph Support for zones as well
- Added support for profiling shared libraries of interest

### Changed
- DoTrace modified for proper tracing only when required
- Engine 2 is now Engine 1 which generates execution and memory profile
- two callgraphs are generated:
    - complete callgraph containing all function and edges
    - simplified callgraph containing important function and edges

### Removed
- detection of static symbols is disabled due to changes in pin 3.0
- Engine 1 removed

### Fixed
- Support for gcc 4.8.1 to 5.4.1

## [ 2.0 ] - [ 2016-07-01 ]
### Added
- Callgraph in json and pdf format

### Changed
- Default for printing communication values is not SI units (1K is 1024)
- Type of communication matrix entry is u64
- 

### Removed
-

### Fixed
- Fixed bug of routine names with recent g++ version
- Better implementation of callpath to ID
- Tested with pin 3.0 version
- Tested with gcc 4.6 to 5.3


## [ 1.3 ] - [ 2015-04-23 ]
### Added
- Support for static symbols
- Support for threshold knob for communication edges
- Simplified communication matrix by removing empty rows/cols

### Changed
- Simplified path printing in output files (e.g. symbol.out)
- Updated manual and example description
- Updated manual with used packages and their example installation

### Removed
- Removed condition of matrix printing with objects

### Fixed
- Fixed Multi-allocation issue in loops and added test
- Fixed the issue with placement of knobs
- Fixed alignment issue in communication matrix


## [ 1.2 ] - [ 2015-03-30 ]
### Added
- Separate execProfile and memProfile.
- Support for tracking zones.
- Added instruction count percentages for functions.
- Added printing of symbol size
- Added printing of number of calls to functions

### Changed
- Updated documentation in Readme.md and manual.pdf.
- Simplified per call output

### Fixed
- Fixed a bug in the hybrid mode.
- Fixed a bug when using code with shared lib.
- Fixed printing issue and other small modifications.


## [ 1.1 ] - [ 2014-12-17 ]
### Added
- First implementation
- This CHANGELOG file.
