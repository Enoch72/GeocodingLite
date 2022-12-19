# GeocodingLite

GeocodingLite is the opensource geocoding server engine for getting the GPS coordinates of a city/location all around the world.  

# About

- Written in C/C++
- Based on [Sqlite FTS5](https://www.sqlite.org/fts5.html) search engine
- Location data from [OpenAddresses](https://openaddresses.io/)
- Powered by [Feroeg](https://feroeg.com) reverse geocoding

# QuickStart (Linux / WSL)

Make sure you have installed the C++ compiler toolchain (GCC or LLVM) and git.

From command prompt console:

Clone the repo from github:
```
git clone https://github.com/Enoch72/GeocodingLite.git
```

Compile sqlite module with the FTS5 module and recommended directives
```
gcc -DSQLITE_THREADSAFE=2 Libs/sqlite3.c -ldl -lm -c -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP -DSQLITE_USE_ALLOCA -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_DIRECT_OVERFLOW_READ -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_ENABLE_MEMSYS5 -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_ENABLE_FTS5 -O3
```

Compile and link GeocodeLite executable with sqlite module
```
g++ -std=c++17 -o geocode_lite geocode_lite.cpp sqlite3.o -pthread -ldl -O3
./geocode_server
```

**WSL users** : *The ip address of the entrypoint of the service will be vary between wsl instances.

If you want use the server from Windows you have to find and substitute it in the first line of /Web/scripts.js file. You have to eventually change the ip port if the service will not listening to 8081.

Find it by launch the command 'ip address' in an instance of the wsl window.
* 


Launch the server:
```
./geocode_server
```

# Using of the service

### Demo page From the browser

Open the browser at 127.0.0.1:8081

Now type the location you want to search.


### Calling the search API engine of the server

 http:127.0.0.1:8081/geocode?query=**search expression**
 
 A simple expression may be: 'IT Milano'
 
 For advanced search please read the [Sqlite FTS5](https://www.sqlite.org/fts5.html) search engine documentation related to 'MATCH' expression.


# Customizations
 At this early stage the service is customizable only by changing the source code.
 
# Docker image
 A docker image of the service will be available on dockerhub
 
# Attribution
 
 If you use this piece of software commercially an attiribution message is required es: "powered by GeocodingLite" in the text search.
