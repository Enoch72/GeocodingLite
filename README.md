# GeocodingLite

GeocodingLite is the opensource geocoding server engine for getting the GPS coordinates of a city/location all around the world.  

# About

- Written in C++
- Based on [Sqlite FTS5](https://www.sqlite.org/fts5.html) search engine
- Location data from [OpenAddresses](https://openaddresses.io/)
- Powered by [Feroeg](https://feroeg.com) reverse geocoding

# QuickStart (Linux / WSL)

Make sure you have installed the C++ compiler toolchain (GCC or LLVM) and git then:

```
git clone https://github.com/Enoch72/GeocodingLite.git
make
./geocode_server
```

# Using of the service

### Demo page From the browser

Open the browser at 127.0.0.1:8081

Now type the location you want to search.

**WSL users** : *The address of the entrypoint of the service is not 127.0.0.1 but will vary with your wsl instance. Find it by launch the command 'ip address' in an intance of the wsl window.* 

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
