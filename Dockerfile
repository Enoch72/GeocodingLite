FROM alpine
RUN apk add --no-cache alpine-sdk
WORKDIR /app
ADD Libs/ /app/Libs
ADD Web/ /app/Web
copy *.c* .
copy *.sqlite .
RUN gcc -DSQLITE_THREADSAFE=2 Libs/sqlite3.c -ldl -lm -c -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP -DSQLITE_USE_ALLOCA -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_DIRECT_OVERFLOW_READ -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_ENABLE_MEMSYS5 -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_ENABLE_FTS5 -O3
RUN g++ -std=c++17 -o GeocodingLite geocode_server.cpp sqlite3.o -pthread -ldl -O3
CMD ["./GeocodingLite"]
EXPOSE 8081
