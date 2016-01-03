#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "dirfiller.h"
#include "utility.h"

void filler_root( void *buf, fuse_fill_dir_t filler )
{
    filler( buf, "Albums", NULL, 0 );
    filler( buf, "Artists", NULL, 0 );
    filler( buf, "Decades", NULL, 0 );
}

void filler_all_albums( PGconn *ytfs_db, void *buf, fuse_fill_dir_t filler )
{
    PGresult *result = PQexec( ytfs_db, "SELECT name FROM tb_album" );
    int ntuples      = PQntuples( result );
    
    int i;
    const char *album_name;
    
    for( i = 0; i < ntuples; i++ )
    {
        album_name = (const char *)PQgetvalue( result, i, 0 );
        filler( buf, album_name, NULL, 0 );
    }
    
    PQclear( result );
}

void filler_all_artists( PGconn *ytfs_db, void *buf, fuse_fill_dir_t filler )
{
    PGresult *result = PQexec( ytfs_db, "SELECT name FROM tb_artist" );
    int ntuples      = PQntuples( result );
    
    int i;
    const char *artist_name;
    
    for( i = 0; i < ntuples; i++ )
    {
        artist_name = (const char *)PQgetvalue( result, i, 0 );
        filler( buf, artist_name, NULL, 0 );
    }
    
    PQclear( result );
}

void filler_all_decades( PGconn *ytfs_db, void *buf, fuse_fill_dir_t filler )
{
    PGresult *result = PQexec( ytfs_db, "SELECT DISTINCT year FROM tb_album" );
    int ntuples      = PQntuples( result );
    
    int i;
    char *decade;
    
    for( i = 0; i < ntuples; i++ )
    {
        decade    = (char *)PQgetvalue( result, i, 0 );
        decade[3] = '0';
        filler( buf, decade, NULL, 0 );
    }
    
    PQclear( result );
}

void filler_albums_from_artist( PGconn *ytfs_db, char *artist, void *buf, fuse_fill_dir_t filler )
{
    const char *param_values[1];
    
    param_values[0]  = artist;
    PGresult *result = PQexecParams(
        ytfs_db,
        "SELECT name \
           FROM tb_album \
          WHERE artist = \
                (SELECT artist FROM tb_artist WHERE name = $1)",
        1,
        NULL,
        param_values,
        NULL,
        NULL,
        0
    );
    int ntuples      = PQntuples( result );
    
    int i;
    const char *album_name;
    
    for( i = 0; i < ntuples; i++ )
    {
        album_name = (const char *)PQgetvalue( result, i, 0 );
        filler( buf, album_name, NULL, 0 );
    }
    
    PQclear( result );
}

void filler_albums_from_decade( PGconn *ytfs_db, char *decade, void *buf, fuse_fill_dir_t filler )
{
    PGresult *result;
    int ntuples;
    
    char query[54];
    sprintf( query, "SELECT name FROM tb_album WHERE year::text LIKE '%s'", decade );
    query[52] = '%';
    
    result  = PQexec( ytfs_db, query );
    ntuples = PQntuples( result );
    
    int i;
    const char *album_name;
    
    for( i = 0; i < ntuples; i++ )
    {
        album_name = (const char *)PQgetvalue( result, i, 0 );
        filler( buf, album_name, NULL, 0 );
    }
    
    PQclear( result );
}

void filler_tracks_in_album( PGconn *ytfs_db, char *album, void *buf, fuse_fill_dir_t filler )
{
    const char *param_values[1];
    
    param_values[0]  = album;
    PGresult *result = PQexecParams(
        ytfs_db,
        "SELECT s.track_number || ' - ' || s.name || '.' || f.extension, \
                f.checksum \
           FROM tb_song s \
           JOIN tb_file f \
             ON s.file = f.file \
          WHERE album = \
                (SELECT album FROM tb_album WHERE name = $1)",
        1,
        NULL,
        param_values,
        NULL,
        NULL,
        0
    );
    int ntuples      = PQntuples( result );
    
    int i;
    const char *song_name;
    
    for( i = 0; i < ntuples; i++ )
    {
        song_name = (const char *)PQgetvalue( result, i, 0 );
        filler( buf, song_name, NULL, 0 );
    }
    
    PQclear( result );
}

