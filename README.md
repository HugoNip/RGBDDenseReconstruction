## Introduction
This project shows creating dense map based on RGBD images.

## Requirements

### Eigen Package (Version >= 3.0.0)
#### Source
http://eigen.tuxfamily.org/index.php?title=Main_Page

#### Compile and Install
```
cd [path-to-Eigen]
mkdir build
cd build
cmake ..
make 
sudo make install 
```
#### Search Installing Location
```
sudo updatedb
locate eigen3
```

default location "/usr/include/eigen3"

### OpenCV
#### Required Packages
OpenCV  
OpenCV Contrib

gcc version: gcc (Ubuntu 5.4.0-6ubuntu1/~16.04.12) 5.4.0 20160609   
g++ version: g++ (Ubuntu 5.4.0-6ubuntu1/~16.04.12) 5.4.0 20160609  
OpenCV with version of both 3.0 and 4.0 tests well.   
(Note: OpenCV will fail to compile with gcc/g++ of 9.2.0 version)

### pcl Package
#### Install
```
sudo apt-get update
sudo apt-get install libpcl-dev pcl-tools
```
#### Solution for the error: cannot find vtkproj4 #1594
```
ln -s /usr/lib/x86_64-linux-gnu/libvtkCommonCore-6.2.so /usr/lib/libvtkproj4.so
```

### octomap Package

```
sudo apt-get install liboctomap-dev octovis
```

## Compile this Project
```
mkdir build
cd build
cmake ..
make 
```

## Run
```
./
./
```



## Reference
[Source](https://github.com/HugoNip/slambook2/tree/master/ch12)
