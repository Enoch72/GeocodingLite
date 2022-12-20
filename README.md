# GeocodingLite

GeocodingLite is the open-source geocoding server engine for getting the GPS coordinates of a city/location all around the world. 

# About

- Written in C/C++
- Based on [Sqlite FTS5](https://www.sqlite.org/fts5.html) search engine
- Location data from [GeoNames](https://www.geonames.org) covers about 100 nations 
- Powered by [Feroeg](https://feroeg.com) reverse geocoding

# QuickStart (Linux / WSL)

Make sure you have installed the C++ compiler toolchain (GCC or LLVM) and git.

From command prompt console:

Clone the repo from github and enter in the folder:
```
git clone https://github.com/Enoch72/GeocodingLite.git
cd GeocodingLite
```

unzip the GeoNames.zip file to obtain the GeoNames.sqlite db file:
```
unzip GeoNames.zip
```


Compile sqlite module with the FTS5 extension and recommended directives
```
gcc -DSQLITE_THREADSAFE=2 Libs/sqlite3.c -ldl -lm -c -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP -DSQLITE_USE_ALLOCA -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_DIRECT_OVERFLOW_READ -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_ENABLE_MEMSYS5 -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_ENABLE_FTS5 -O3
```

Compile and link GeocodeLite executable with sqlite module
```
g++ -std=c++17 -o geocode_lite geocode_lite.cpp sqlite3.o -pthread -ldl -O3
```

**WSL users** : *The IP address of the entry point of the service will vary between Windows WSL instances.
If you want to use the server from Windows you have to find and substitute it in the first line of /Web/scripts.js file.
Find it by launching the command 'IP address' in an instance of the WSL window. You have to eventually change the IP port if the service will not listen to 8081 (if you have modified the source code).*

Launch the server:
```
./geocode_server
```

# Using of the service

### Demo page From the browser

Open the browser at 127.0.0.1:8081 **WSL users**: *you have to get the IP address of the WSL instance -- read note below* 

Now type the location you want to search.


### Calling the search API engine of the server

 http:127.0.0.1:8081/geocode?query=**search expression**
 
 A simple expression may be: 'IT Milano'
 
 For advanced search please read the [Sqlite FTS5](https://www.sqlite.org/fts5.html) search engine documentation related to the 'MATCH' expression.

# Customizations
 At this early stage, the service is customizable only by changing the source code.
  
# Attribution
 
If you use this piece of software, or a modified version of this, please be fair to find the appropriate way to attribute the work, like leaving the attribution message in the text search in the demo like: "... search powered by GeocodingLite" so the final user can read this.
