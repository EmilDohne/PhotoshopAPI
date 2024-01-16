# PhotoshopAPI

## Project

- [X] Generate CMake project for PhotoshopAPI
- [X] Include blosc2 compression library
- [X] Add README (initial)
- [ ] Add examples on how to interact with the API (ideally cover as many cases as possible)

## Testing

- [X] Add wide-range of test cases for testing based on photoshop files (both psd and psb)
- [ ] Add unit testing for low-level components such as reading from a binary file
 
## Nice to haves

- [ ] An asynchronous/threaded function that allows us to quickly get the percentage read/write completion while that action is in course

## PhotoshopFile

### Read
- [X] Read File Header
- [X] Read ColorModeData
- [X] Read Image Resources
- [x] Read Layer and Mask information
    - [X] Read Global Layer Mask Info
    - [X] Read Layer Records
        - [X] Add parsing of tagged blocks 
    - [x] Read Channel Image Data
        - [x] Compress using blosc2
    - [x] Read Additional Layer Information
- [ ] Read Merged Image Data Section
    - [ ] Compress using blosc2

### Write

#### Parsing from LayeredFile -> PhotoshopFile
- [X] Parse File Header
- [X] Parse ColorModeData
- [X] Parse Image Resources
- [ ] Parse Layer and Mask information
    - [ ] Parse Global Layer Mask Info
    - [ ] Parse Layer Records
    - [ ] Parse Channel Image Data
    - [ ] Parse Additional Layer Information
- [ ] Parse Merged Image Data Section (?)


#### Write the actual data
- [ ] Write File Header
- [ ] Write ColorModeData
- [ ] Write Image Resources
- [ ] Write Layer and Mask information
    - [ ] Write Global Layer Mask Info
    - [ ] Write Layer Records
    - [ ] Write Channel Image Data
    - [ ] Write Additional Layer Information
- [ ] Write Merged Image Data Section (?)