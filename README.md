# Kinemic SDK C++ Sample

This repository contains a sample application showcasing the usage of the Kinemic SDK to integrate gesture control into a C++ application.
For more information about the SDK and our solutions for gesture interaction visit us at: https://kinemic.com.

## Dependencies (Linux)

The sample is built using CMake and PkgConfig for dependency management.

### Kinemic Gesture SDK

You will need to install our Gesture SDK. On Ubuntu or Debian based systems we
provide an apt repository:

```
deb https://kinemic.jfrog.io/kinemic/debian <distribution> main
```

Replace `distribution` with your Debian or Ubuntu distribution (we currently
provide packages for `jessie, stretch, xenial, artful, bionic`) and either
include the above line in your `/etc/apt/sources.list` or in
`/etc/apt/sources.list.d/kinemic.list`.

Afterwards run:

```
wget -qO- https://kinemic.jfrog.io/kinemic/api/gpg/key/public | sudo apt-key add -
sudo apt update
sudo apt install kinemic-sdk-dev kinemic-sdk-static
```

You can replace `kinemic-sdk-static` with `kinemic-sdk` if you want to link our
Gesture SDK as a shared library.

### Libblepp BLE library

The sample application also uses [libblepp](https://github.com/edrosten/libblepp) to
communicate with our Bands via BLE. Make sure you have this installed.

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

## Engine Application

This sample application instanciates a kinemic::KinemicEngine. 

It then registers various listeners on this instance to receive connection state updates and
gesture events.

Afterwards it tries to connect to the Kinemic Band with the strongest signal in
range.

The Application disconnects connected sensors and releases ressources when the user leaves the app.
