//  ------------------------------------------------------------------
//  SqliteDB - simple single header C++ wrapper for sqlite
//  Copyright (c) 2022 Castelli Giovani.
//  MIT License
//  Developed for https://github.com/Enoch72/GeocodingLite.git

#pragma once
#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include <mutex>
#include "sqlite3.h"
#include <chrono>
#include <atomic> 
#include <map>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class SqliteDB 
  {
    public:
    sqlite3    *db_handle;
    std::chrono::time_point<std::chrono::steady_clock> creation_time;
    std::atomic<void *> UserData;
    private:
    std::map<std::string,sqlite3_stmt *> stmts_map;
    std::string sqlite_path;
    int   sqlite_open_flags;
    bool  Opened;
     

// TODO check free objects allocated in 
    public:
    
    bool IsActive()       { return Opened; };
    std::string &DBPath() { return sqlite_path; };

    SqliteDB(std::string db_path,int open_flags, int db_cache_size_mb):
     sqlite_path(db_path), sqlite_open_flags(open_flags), db_handle(NULL), creation_time( std::chrono::steady_clock::now()), UserData(0)
    {
      Opened = false;
     
      if ( !(SQLITE_OPEN_CREATE & sqlite_open_flags) &&  !fs::exists(sqlite_path) )
       {
        fprintf (stderr, "Sqlite file to open not exists! : '%s\r\n", db_path.c_str());
        db_handle = NULL;
        return;
       } 

      int ret = sqlite3_open_v2( db_path.c_str(), &db_handle, sqlite_open_flags, NULL);
      if (ret != SQLITE_OK)
       {
        fprintf (stderr, "Sqlite open error: '%s': %s\r\n", db_path.c_str(),
        sqlite3_errmsg (db_handle));
        sqlite3_close (db_handle);
        db_handle = NULL;
       }
        else
       {
         if (sqlite_open_flags & SQLITE_OPEN_READWRITE)
          {
            sql_exec ("PRAGMA journal_mode = OFF");
            sql_exec ("PRAGMA synchronous = 0");
            sql_exec ( std::string("PRAGMA cache_size=" + std::to_string(-db_cache_size_mb)).c_str() );
          }  
       }
       Opened= (ret == SQLITE_OK);
    }

    ~SqliteDB()
    {
      for (auto &pair : stmts_map)
       sqlite3_finalize(pair.second); 
      if (db_handle)
       sqlite3_close (db_handle);
       
    }
    
    sqlite3_stmt *GetSqliteStmt(const char *stmt_name)
    {
      return stmts_map[stmt_name]; 
    }

    sqlite3_stmt *sql_prepare(const char *stmt_name,const char *sql_statement)
    {
      sqlite3_stmt *stmt;
      int ret = sqlite3_prepare_v2 (db_handle, sql_statement, strlen (sql_statement), &stmt, NULL);
      if (ret == SQLITE_OK)
      {
       stmts_map[stmt_name]=stmt;
       return stmt;
      }
      return NULL;
    }

    sqlite3_stmt *sql_prepare(const char *stmt_name,std::string &sql_statement)
    {
      return sql_prepare(stmt_name,sql_statement.c_str());
    }

    sqlite3_stmt *sql_prepare(const char *sql_statement)
    {
      return sql_prepare(std::string("sql_stmt_"+std::to_string(stmts_map.size()+1)).c_str(),sql_statement);
    }

    bool sql_exec (const char *sql_text)
    {
      char *err_msg;
      int ret = sqlite3_exec (db_handle, sql_text, NULL, NULL, &err_msg);
      if (ret != SQLITE_OK)
        {
          fprintf (stderr, "Sql error: %s\r\n", sql_text);
          sqlite3_free (err_msg);
          return false;
        }
      return true;  
    }
  
  };
  #endif