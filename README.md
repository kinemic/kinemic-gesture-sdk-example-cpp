# Kinemic SDK C++ Sample

This repository contains a sample application showcasing the usage of the Kinemic SDK to integrate gesture control into a C++ application.
For more information about the SDK and our solutions for gesture interaction visit us at: https://kinemic.com.

# Linux

## Dependencies

The sample is built using CMake and PkgConfig for dependency management.

### Kinemic Gesture SDK

You will need to install our Gesture SDK.

Check out our [installation instructions.](https://kinemic.com/en/developerarea/installation/#linux)

### Libblepp BLE library

The sample application also uses [libblepp](https://github.com/edrosten/libblepp) to
communicate with our Bands via BLE. Make sure you have this installed.

```
$ git clone https://github.com/edrosten/libblepp.git
$ cd libblepp
$ ./configure --prefix=/usr
$ make
$ sudo make install  # or use checkinstall if you want to integrate it into package management
```

## Building

After cloning the repository you can build the application:

```
mkdir build && cd build
cmake ..
make
```

## Running the application

BLE (searching) access on Linux requires elevated permissions.

If you try to run the built `kinemic-gesture-sdk-example-cpp` executable without
these permissions you will get an error message: `Was not able to start scanner. Scanning requires 'cap_net_raw,cap_net_admin+eip' permissions`

You can either run the application as root

```
sudo ./kinemic-gesture-sdk-example-cpp
```

or you can elevate the permissions of the
executable using

```
sudo setcap 'cap_net_raw,cap_net_admin+eip' kinemic-gesture-sdk-example-cpp
```

During the execution of the application you will also see the following output:

```
error 1555338171.144329: Error on line: 296 (src/blestatemachine.cc): Operation now in progress
```

This is *not* an error message, it is in fact a message from [libblepp](https://github.com/edrosten/libblepp)
to inform you that a connection is being established.

# Windows

## Dependencies

The sample is built using CMake and Nuget for dependency management.

### Kinemic Gesture SDK

You will need to install our Gesture SDK. Especially, you have to setup a valid Nuget respository that provices our CPP Windows package.

Check out our [installation instructions.](https://kinemic.com/en/developerarea/installation/#windows)

## Building

After cloning the repository you can build the application using Visual Studio.

Open the project as a CMake project (using `File>Open>Cmake..` and targeting the `CMakeLists.txt`), afterwards it will be populated with two targets:

```
kinemic_sdk_nuget
kinemic-gesture-sdk-example-cpp.exe
```

Build the `kinemic_sdk_nuget` target first, this will fetch the Kinemic Gesture SDK as a dependency.
Afterwards you can build the main executable target.

# Engine Application

This sample application instanciates a kinemic::KinemicEngine.

It then registers various listeners on this instance to receive connection state updates and
gesture events.

Afterwards it tries to connect to the Kinemic Band with the strongest signal in
range.

The Application disconnects connected sensors and releases ressources when the user leaves the app.
