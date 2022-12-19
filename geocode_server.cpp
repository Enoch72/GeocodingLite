
//  GEOCODE LITE SERVER  --- Copyright (c) 2022  Giovanni Castelli
//  *** THE CODE IN THIS FILE IS UNDER MIT LICENSE ***
//  Nevertheless please be fair and find a way to attribute properly this work

#include <iostream>
#include <chrono>
#include <thread>

#include "Libs/httplib.hpp"
using namespace httplib;
//  ------------------------------------------------------------------
//  HTTPLIB --- Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//  https://github.com/yhirose/cpp-httplib

#include "Libs/json/single_include/nlohmann/json.hpp"
//  ------------------------------------------------------------------
//  JSON -- Copyright (c) Niels Lohmann
//  MIT License
//  https://github.com/nlohmann/json

#include "Libs/SqliteDB.hpp"
//  ------------------------------------------------------------------
//  SqliteDB.hpp -- Simple one header sqlite C++ wrapper
//  MIT License by Giovanni Castelli - Feroeg.com
//  ------------------------------------------------------------------



SqliteDB *FT5_Sqlite_Session(std::string &dbpath,int sqlite_flags,int cache_MB)
{ SqliteDB * sqlite_session = new SqliteDB( dbpath, sqlite_flags, cache_MB );
  /* The main work is done by this query --> See last lines of this file for more info about query*/
  sqlite_session->sql_prepare("ft5query",
  "with A as (select ROWID,substr(rank,1,8) as RANK,* from txt5 where txt5 match ? order by rank DESC LIMIT 20) "
  "select A.*,b.POSTAL_CODE,b.LATITUDE,B.LONGITUDE from A left join allCountries b on A.ROWID = b.ROWID");
  return sqlite_session;
}

class HandlePool
{
  public: // TODO private 
  std::mutex Locker;
  std::map< void *, bool> HandleMap;
  public:
   
  void  InsertHandle(void *Handle) 
    {
      const std::lock_guard<std::mutex> lock(Locker);
      HandleMap.insert(std::pair< void *, bool>(Handle,false));
    }
  
  void *AcquireHandle() // Acquire an handle to a search instance 
    {
       void *result = NULL;
       while (result == NULL)
       {
          {
          const std::lock_guard<std::mutex> lock(Locker);
          for (auto pairs = HandleMap.begin(); pairs != HandleMap.end(); ++pairs)
            {
              if (pairs->second==false)
                {
                  result = pairs->first;
                  pairs->second = true;
                  break;
                }
            }
          }
          if (result==NULL)
           std::this_thread::yield();
       }
       return result;
    }


  void  ReleaseHandle(void *Handle)
    {
      const std::lock_guard<std::mutex> lock(Locker);
      for (auto pairs = HandleMap.begin(); pairs != HandleMap.end(); ++pairs)
        {
          if (pairs->first==Handle)
            {
              pairs->second = false;
              break;
            }
        }
    }

};

inline std::string& rtrim(std::string& s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

void CommaToSpacesAndTrim(std::string &s)
{
  for (int len = s.length()-1; len>=0; len--)
   if (s[len] == ',' ) 
      s[len]=' ';
     
  rtrim(s," ");
}

bool geocode_query(SqliteDB *DB_Handle,std::string &search_query,std::string &JSON_Result)
{
   using nlohmann::json;
   sqlite3_stmt *stmt = DB_Handle->GetSqliteStmt("ft5query");
   if (stmt)
   {
    sqlite3_bind_text(stmt,1,search_query.c_str(),search_query.length(),NULL);
    json json_result;
    std::string last_line;
    while ( sqlite3_step(stmt) == SQLITE_ROW )
     {
      json json_line;
      std::string line,coords,last_column;
      for (int i=1; i!=11; ++i) 
        {
         std::string column(std::string((const char*) (sqlite3_column_bytes(stmt,i) == 0 ? (const unsigned char*)"" : sqlite3_column_text(stmt,i))));
        
         if (i>=2 && i<=6)
          { 
           if (last_column!=column) // To not display text redundances EX: ..New York,New York,New York.. 
            {
             line       += column + ",";
             last_column = column;
            }
          }
         
         if (column.length()==0)
          continue;

         json_line[sqlite3_column_name(stmt,i)] = column;
        }

      rtrim(line,",");

      if (line!=last_line)
      { 
       last_line = line;
       json_line["TEXT"]  = line;
       json_result.push_back(json_line); 
      } 
     }
    json json_result2;
    json_result2["attribution"]= "GeocodeLite Server. Powered by feroeg.com - Data from GeoNames";
    json_result2["results"]=json_result;
     
    sqlite3_clear_bindings (stmt);
    sqlite3_reset          (stmt);
    JSON_Result = json_result2.dump();
    return true;
   }
  return false; 
}

bool ValidateGeocodeRequest(const Request & req, std::string &result)
{
   std::string request = req.get_param_value("query");
   std::cout << "Request q=" << request << std::endl;
   
   CommaToSpacesAndTrim(request);
   
   if ( request.length()>=2 )
    {     
     result = request + '*';
     return true;
    }  
  else
    {
     result=""; 
     return false;
    }
}


int main (int argc, char *argv[]) 
{
  
  
  HandlePool *OSMPool;
  HandlePool *OpenAddressesPool;

  std::string geocode_file = "./GeoNames.sqlite";
  
 /**
  if (argc==3 && !strcmp(argv[1],"ini_path"))
    {
      INIReader   IniFile(argv[2]);
      if (IniFile.ParseError())
       {
        std::cout << "IniFile - Error on parse ini file - " << argv[2] << std::endl;
        exit(1);
       }
      
      georef_file   = IniFile.Get("FEROEG_PATHS", "GEOREF_FILE"    , osm_dir);
      
    }
    **/

  std::cout << "Starting up geocode server. " << std::endl;
  std::cout << "Geocode db  file:" << geocode_file << std::endl;
  int numpool = std::thread::hardware_concurrency();
  
  std::cout << "Creating a Sqlite pool of " << numpool << " search instances." << std::endl;
  OSMPool = new HandlePool();
  for (int i=0; i<numpool ; i++)
  { 
     SqliteDB *handle = FT5_Sqlite_Session(geocode_file, SQLITE_OPEN_READONLY, 128 ); 
     if (!handle)
      { 
        std::cout << "Fatal : cannot open " << geocode_file << std::endl ;
        exit(404);
      }
     OSMPool->InsertHandle ( handle );
  }
  std::cout << "Creating an http server instance with " << numpool << " worker threads." << std::endl;
  Server svr(numpool);
 
  svr.Get("/geocode", [OSMPool](const Request & req, Response &res) {
   auto clock      = std::chrono::system_clock::now();
   std::string JSON_Result,search_query; 
   std::cout << "GeocodeRequest: "  << req.path <<  std::endl;
   if ( ValidateGeocodeRequest(req,search_query) )
   {
     SqliteDB *DB_Handle = (SqliteDB *) OSMPool->AcquireHandle();
     geocode_query(DB_Handle,search_query,JSON_Result); 
     OSMPool->ReleaseHandle(DB_Handle);
     std::cout << JSON_Result << std::endl;
     int count = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-clock).count();
     std::cout << "Query resolved in " << std::dec << count << " ms" << std::endl;    
     res.set_header("Access-Control-Allow-Origin","*");
     res.set_content(JSON_Result, "application/json; charset=utf-8");
   }
  } ); // Svr.Get

  // Configuring path for serve web pages/files
  std::cout << "Mount web pages at the relative path of: ./" << std::endl;
  svr.set_mount_point("/","./Web");

  // Starts web server
  std::cout << "Starting Web server on port 8081" << std::endl;
  std::cout << "API ENDPOINT: HTTP:x.x.x.x:8081/geocode?query=[search expression]" << std::endl; 
  if (!svr.listen("0.0.0.0", 8081) )
    {
       std::cout << "Failed!" << std::endl;
       exit(1);
    }  
  
}

/* HOW TO COMPILE FROM COMMAND LINE:

You can compile from command line the server using this two commands:

1) Compile sqlite module 
gcc -DSQLITE_THREADSAFE=2 Libs/sqlite3.c -ldl -lm -c -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP -DSQLITE_USE_ALLOCA -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_DIRECT_OVERFLOW_READ -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_ENABLE_MEMSYS5 -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_ENABLE_FTS5 -O3

2) Compile and link GeocodeLite executable
g++ -std=c++17 -o geocode_lite geocode_lite.cpp sqlite3.o -pthread -ldl -O3
 
 **/



/** SQLITE FT5 
 
 This server uses this SQL query to resolve geocoding locations:

 with A as (select ROWID,substr(rank,1,8) as RANK,* from txt5 where txt5 match [search_query] order by rank DESC LIMIT 20) 
 select A.*,b.POSTAL_CODE,b.LATITUDE,B.LONGITUDE from A left join allCountries b on A.ROWID = b.ROWID

--- ABOUT RESULTSET AND COLUMNS OF THE QUERY: ---
The resultset for the above query and search_query = "IT Lurat*" is the following:

ROWID  | RANK    |NATION| PLACE NAME     | ADMIN_NAME1 | ADMIN_NAME2 | ADMIN_NAME3 | POSTAL_CODE | LATITUDE | LONGITUDE
437653  -18.78..	  IT	  Lurate Caccivio	 Lombardia	    Como		                    22075	        45.767	    8.999

--- ABOUT JSON RESPONSE ENDPOINT (/Geocode?query=[search_query])

The resultset returned by the API endpoint is in JSON format. The dataset wievwd in the last paragraph will be formatted: 

{
   "attribution":"GeocodeLite Server. Powered by feroeg.com - Data from OpenAddresses",
   "results":[
      {
         "ADMIN_NAME1":"Lombardia",
         "ADMIN_NAME2":"Como",
         "LATITUDE":"45.767",
         "LONGITUDE":"8.999",
         "NATION":"IT",
         "PLACE_NAME":"Lurate Caccivio",
         "POSTAL_CODE":"22075",
         "RANK":"-18.7832",
         "TEXT":"IT,Lurate Caccivio,Lombardia,Como"
      }
   ]
}

The "results" field is an array of fields, like above. Only 1 result is showed for simplicity. 
The data in the "TEXT" field is an aggregate of the NATION,PLACE_NAME,ADMIN_NAME(1,2,3) fields. It's used
by the HTML demo page of the server for displaying the search result(s).
The "LATITUDE" and "LONGITUDE" fields are used for center the view of the map in the appropriate position. 
NOTE: A one or more fields will be omitted in the JSON object if the own value is NULL (as ADMIN_NAME3, for example). 

**/
