# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [ 0.4 ] - [ 2015-10-08 ]
### Added
- 

### Changed
- Default for printing communication values is not SI units (1K is 1024)
- Type of communication matrix entry is u64

### Removed
- 

### Fixed
- Fixed bug of routine names with recent g++ version
- Better implementation of callpath to ID


## [ 0.3 ] - [ 2015-04-23 ]
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


## [ 0.2 ] - [ 2015-03-30 ]
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


## [ 0.1 ] - [ 2014-12-17 ]
### Added
- First implementation
- This CHANGELOG file.
