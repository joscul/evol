
## Build
Check out the code and open a terminal at the location.
### Linux
```
mkdir build
cd build
cmake ..
make
./evol
```

## Mac OS (xcode)
```
mkdir xcode
cd xcode
cmake -G Xcode ..
```
This will create an Xcode project in the `xcode` folder. This command will open the project in xcode.
```
open evol.xcodeproj
```
Just press play in xcode to build and run the examples.

## Windows (visual studio community)
Just clone the repository inside visual studio, it has `cmake` support.
You might need to go to the menu `Project -> Delete Cache and Reconfigure` to re-run the cmake config.
