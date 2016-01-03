#include <openssl/md5.h>
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "utility.h"

inline int error( char *func )
{
    int retval = -errno;
    syslog( LOG_NOTICE, "Error in %s: %s", func, strerror( errno ) );
    return retval;
}

int get_slash_count( const char *path )
{
    int i;
    
    int count = 0;
    int lim   = strlen( path );
    
    for( i = 0; i < lim; i++ )
    {
        if( path[i] == '/' )
            count++;
    }
    
    return count;
}

int last_index_of( const char *path, char c )
{
    int i;
    
    int retval = -1;
    int lim    = strlen( path );
    
    for( i = 0; i < lim; i++ )
    {
        if( path[i] == c )
            retval = i;
    }
    
    return retval;
}

int python( char *script_file, int argc, char *argv[] )
{
    FILE *py;
    char *py_arg[argc + 1];
    char fname[PATH_MAX];
    int i;
    int py_ret = 0;
    
    // 1. Get file for reading
    full_bin_python_path( fname, script_file );
    py = fopen( fname, "r" );
    
    // 2. Build args array
    py_arg[0] = (char *)script_file;
    
    for( i = 0; i < argc; i++ )
        py_arg[i + 1] = argv[i];
    
    // 3. Run if possible
    if( py )
    {
        PySys_SetArgv( argc + 1, py_arg );
        py_ret = PyRun_SimpleFile( py, fname );
        
        if( py_ret < 0 )
            syslog( LOG_NOTICE, "Python script encountered an exception!" );
        
        fclose( py );
    }
    else
        return -1;
    
    return py_ret;
}

int get_checksum_and_extension( PGconn *ytfs_db, char buf[32], char extension[PATH_MAX], const char *path )
{
    int i, ntuples;
    const char *param_values[1];
    char *pgbuf_checksum;
    char *pgbuf_extension;
    
    // 1. Get a path like "Album/# - Song.Extension"
    int index  = -1;
    int next   = -1;
    int lim    = strlen( path );
    
    for( i = 0; i < lim; i++ )
    {
        if( path[i] == '/' )
        {
            index  = next;
            next   = i;
        }
    }
    
    // 2. Get that song's checksum
    param_values[0]  = path + index + 1;
    PGresult *result = PQexecParams(
        ytfs_db,
        "SELECT f.checksum, \
                '.' || f.extension \
           FROM tb_file f \
           JOIN tb_song s \
             ON s.file = f.file \
           JOIN tb_album a \
             ON s.album = a.album \
          WHERE a.name || '/' || s.track_number || ' - ' || s.name || '.' || f.extension = $1",
        1,
        NULL,
        param_values,
        NULL,
        NULL,
        0
    );
    ntuples      = PQntuples( result );
    
    // 3. If we couldn't find one, return -1
    if( ntuples <= 0 )
    {
        PQclear( result );
        return -1;
    }
    
    // 4. Put the checksum in a temporary buffer
    pgbuf_checksum  = (char *)PQgetvalue( result, 0, 0 );
    pgbuf_extension = (char *)PQgetvalue( result, 0, 1 );
    PQclear( result );
    
    // 5. If the checksum is empty, return -1
    if( !strcmp( pgbuf_checksum, "" ) || !strcmp( pgbuf_extension, "" ) )
        return -1;
    
    // 6. Copy the checksum into the buffer
    strcpy( buf, pgbuf_checksum );
    strcpy( extension, pgbuf_extension );
    return 0;
}

int get_file_size( PGconn *ytfs_db, const char *path )
{
    int i, ntuples;
    const char *param_values[1];
    const char *pgbuf;
    
    // 1. Get a path like "Album/# - Song.Extension"
    int index  = -1;
    int next   = -1;
    int lim    = strlen( path );
    
    for( i = 0; i < lim; i++ )
    {
        if( path[i] == '/' )
        {
            index  = next;
            next   = i;
        }
    }
    
    // 2. Get that song's checksum
    param_values[0]  = path + index + 1;
    PGresult *result = PQexecParams(
        ytfs_db,
        "SELECT f.size \
           FROM tb_file f \
           JOIN tb_song s \
             ON s.file = f.file \
           JOIN tb_album a \
             ON s.album = a.album \
          WHERE a.name || '/' || s.track_number || ' - ' || s.name || '.' || f.extension = $1",
        1,
        NULL,
        param_values,
        NULL,
        NULL,
        0
    );
    ntuples      = PQntuples( result );
    
    // 3. If we couldn't find one, return -1
    if( ntuples <= 0 )
    {
        PQclear( result );
        return -1;
    }
    
    // 4. Put the checksum in a temporary buffer
    pgbuf = (const char *)PQgetvalue( result, 0, 0 );
    PQclear( result );
    
    // 5. If the checksum is empty, return -1
    if( !strcmp( pgbuf, "" ) )
        return -1;
    
    return atoi( pgbuf );
}

inline void full_fs_path( char buf[PATH_MAX], const char *path )
{
    strcpy( buf, fs_path );
    strcat( buf, path );
}

inline void full_bin_path( char buf[PATH_MAX], const char *path )
{
    strcpy( buf, bin_path );
    strcat( buf, path );
}

inline void full_bin_python_path( char buf[PATH_MAX], char *py )
{
    full_bin_path( buf, "/python/" );
    strcat( buf, py );
}

void md5( char *filename, char buf[32] )
{
    FILE *file = fopen( filename, "rb" );
    unsigned char c[MD5_DIGEST_LENGTH], data[1024];
    int i, bytes;
    MD5_CTX mdContext;

    MD5_Init( &mdContext );
    
    while( ( bytes = fread( data, 1, 1024, file ) ) != 0 )
        MD5_Update( &mdContext, data, bytes );
    
    MD5_Final( c, &mdContext );
    
    for( i = 0; i < MD5_DIGEST_LENGTH; i++ )
        sprintf( buf + i, "%02x", c[i] );

    fclose( file );
}

inline int is_valid_file( const char *path )
{
    int slash_count = get_slash_count( path );
    
    return ( slash_count == 3 && !memcmp( path, "/Albums", 7 ) )
	    || ( slash_count == 4 && !memcmp( path, "/Artists", 8 ) )
	    || ( slash_count == 4 && !memcmp( path, "/Decades", 8 ) );
}

inline int is_valid_folder( const char *path )
{
    int slash_count = get_slash_count( path );
    
    return !strcmp( path, "/" )
	    || ( slash_count < 3 && !memcmp( path, "/Albums", 7 ) )
	    || ( slash_count < 4 && !memcmp( path, "/Artists", 8 ) )
	    || ( slash_count < 4 && !memcmp( path, "/Decades", 8 ) );
}

