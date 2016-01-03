#define FUSE_USE_VERSION 26
#define _GNU_SOURCE
#define DEBUG

#include <dirent.h>
#include <fcntl.h>
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <syslog.h>
#endif

#include "dirfiller.h"
#include "utility.h"
#include "ytfs.h"

char bin_path[PATH_MAX];
char fs_path[PATH_MAX];

static PGconn *ytfs_db;

static struct fuse_operations ytfs_oper = {
    .getattr     = ytfs_getattr,
    .unlink      = ytfs_unlink,
    .truncate    = ytfs_truncate,
    .open        = ytfs_open,
    .read        = ytfs_read,
    .write       = ytfs_write,
    .release     = ytfs_release,
    .readdir     = ytfs_readdir,
    .init        = ytfs_init,
    .destroy     = ytfs_destroy,
    .create      = ytfs_create,
    .fgetattr    = ytfs_fgetattr,
    .utimens     = ytfs_utimens,
};

int ytfs_getattr( const char *path, struct stat *stbuf )
{
    int checksum_success;
    char fpath[PATH_MAX], checksum[32], extension[PATH_MAX];
    
    struct stat realstat;
	memset( stbuf, 0, sizeof( struct stat ) );
	
	if( is_valid_folder( path ) )
    {
	    stbuf->st_mode  = S_IFDIR | 0777;
	    stbuf->st_nlink = 2;
	    stbuf->st_uid   = getuid();
        stbuf->st_gid   = getgid();
    }
    else if( is_valid_file( path ) )
	{
        checksum_success = get_checksum_and_extension( ytfs_db, checksum, extension, path );
            
        if( checksum_success < 0 )
            return -ENOENT;
            
	    full_fs_path( fpath, "/" );
	    strcat( fpath, checksum );
	    strcat( fpath, extension );
	    
	    if( !lstat( fpath, &realstat ) )
	        memcpy( stbuf, &realstat, sizeof( realstat ) );
	    else
	    {   
            stbuf->st_mode    = S_IFREG | 0777;
            stbuf->st_nlink   = 1;
            stbuf->st_uid     = getuid();
            stbuf->st_gid     = getgid();
            stbuf->st_size    = get_file_size( ytfs_db, path );
            stbuf->st_blksize = 4096;
            
            if( stbuf->st_size < 0 )
                stbuf->st_size = 0;
	    }
    }
	else
	    return -ENOENT;
	
	return 0;
}

int ytfs_unlink( const char *path )
{
    int checksum_success, py_ret;
    char fpath[PATH_MAX], checksum[32], extension[PATH_MAX];
    char *pyargs[1];
    
    if( is_valid_file( path ) )
	{
	    checksum_success = get_checksum_and_extension( ytfs_db, checksum, extension, path );
	    
	    if( checksum_success < 0 )
	        return -ENOENT;
        
        full_fs_path( fpath, "/" );
	    strcat( fpath, checksum );
	    strcat( fpath, extension );
	    
	    pyargs[0] = checksum;
        py_ret = python( "delete_file.py", 1, pyargs );
	    
	    if( !py_ret )
            unlink( fpath );
        else
            return -ENOENT;
        
        return 0;
	}
    else
        return -ENOENT;
}

int ytfs_truncate( const char *path, off_t newsize )
{
    int retval, checksum_success;
    char fpath[PATH_MAX], checksum[32], extension[PATH_MAX];
    
    if( is_valid_file( path ) )
    {
        checksum_success = get_checksum_and_extension( ytfs_db, checksum, extension, path );
            
        if( checksum_success < 0 )
            return -ENOENT;
        
        full_fs_path( fpath, "/" );
        strcat( fpath, checksum );
        strcat( fpath, extension );
    }
    else
        full_fs_path( fpath, path );
    
    retval = truncate( fpath, newsize );
    
    if( retval < 0 )
        retval = error( "ytfs_truncate" );
    
    return retval;
}

int ytfs_open( const char *path, struct fuse_file_info *fi )
{
    static char oldpath[PATH_MAX], newpath[PATH_MAX], checksum[32], extension[PATH_MAX];
    
    int retval = 0;
    
    int fd, checksum_success;
    char fpath[PATH_MAX];
    char *pyargs[2];
    
    if( is_valid_file( path ) )
    {
        strcpy( oldpath, newpath );
        strcpy( newpath, path );
        
        full_fs_path( fpath, "/" );
        
        if( strcmp( oldpath, newpath ) )
        {
            pyargs[0] = fpath;
            pyargs[1] = (char *)path;
            
            syslog( LOG_NOTICE, "Arg 0: %s ; Arg 1: %s", pyargs[0], pyargs[1] );
            
            python( "download_file.py", 2, pyargs );
            
            checksum_success = get_checksum_and_extension( ytfs_db, checksum, extension, path );
            
            if( checksum_success < 0 )
                return -ENOENT;
        }
        
        strcat( fpath, checksum );
        strcat( fpath, extension );
        
        fd = open( fpath, fi->flags );
        
        if( fd < 0 )
            retval = error( "ytfs_open" );
    }
    else
    {
        full_fs_path( fpath, path );
        fd = open( fpath, fi->flags );
        
        if( fd < 0 )
            retval = error( "ytfs_open" );
    }
    
    fi->fh = fd;
    return retval;
}

int ytfs_read( const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi )
{
    int retval = pread( fi->fh, buf, size, offset );
    
    if( retval < 0 )
        retval = error( "ytfs_read" );
    
    return retval;
}

int ytfs_write( const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi )
{
    int ext;
    char fpath[PATH_MAX], realname[PATH_MAX], hash[32];
    char *hashed_name;
    char *pyargs[1];
    
    int retval = pwrite( fi->fh, buf, size, offset );
    
    if( retval < 0 )
        retval = error( "ytfs_write" );
    else if( retval < 4096 )
    {
        full_fs_path( fpath, path );
        
        ext         = last_index_of( path, '.' );
        hashed_name = malloc( strlen( path + ext ) + 32 );
        
        md5( fpath, hash );
        sprintf( hashed_name, "/%s%s", hash, path + ext );
        full_fs_path( realname, hashed_name );
        free( hashed_name );
        
        rename( fpath, realname );
        fchmod( fi->fh, 0777 );
            
        pyargs[0] = realname;
        python( "sync_file.py", 1, pyargs );
    }
    
    return retval;
}

int ytfs_release( const char *path, struct fuse_file_info *fi )
{
    return close( fi->fh );
}

int ytfs_opendir( const char *path, struct fuse_file_info *fi )
{
    int retval = 0;
    DIR *dp;
    char fpath[PATH_MAX];
    
    full_fs_path( fpath, path );
    dp = opendir( fpath );
    
    if( !dp )
        retval = error( "ytfs_opendir" );
    
    fi->fh = (intptr_t)dp;
    
    return retval;
}

int ytfs_readdir( const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	char *path_dup = strdup( path );
    char *subdir   = strtok( path_dup, "/" );
    
	char *album, *artist, *decade;
	
	filler( buf, ".", NULL, 0 );
    filler( buf, "..", NULL, 0 );
	
	if( !strcmp( path, "/" ) )
	    filler_root( buf, filler ); 
    else
    {
        if( !strcmp( subdir, "Albums" ) )
        {
            album = strtok( NULL, "/" );
            
            if( album )
                filler_tracks_in_album( ytfs_db, album, buf, filler );
            else
                filler_all_albums( ytfs_db, buf, filler );
        }
        else if( !strcmp( subdir, "Artists" ) )
        {
            artist = strtok( NULL, "/" );
            
            if( artist )
            {
                album = strtok( NULL, "/" );
                
                if( album )
                    filler_tracks_in_album( ytfs_db, album, buf, filler );
                else
                    filler_albums_from_artist( ytfs_db, artist, buf, filler );
            }
            else
                filler_all_artists( ytfs_db, buf, filler );
        }
        else if( !strcmp( subdir, "Decades" ) )
        {
            decade = strtok( NULL, "/" );
            
            if( decade )
            {
                album = strtok( NULL, "/" );
                
                if( album )
                    filler_tracks_in_album( ytfs_db, album, buf, filler );
                else
                    filler_albums_from_decade( ytfs_db, decade, buf, filler );
            }
            else
                filler_all_decades( ytfs_db, buf, filler );
        }
        else
            return -ENOENT;
    }
    
    return 0;
}

void* ytfs_init( struct fuse_conn_info *conn )
{
    #ifdef DEBUG
    openlog( "ytfs_log", LOG_PID | LOG_CONS, LOG_USER );
    syslog( LOG_NOTICE, "YTFS initialized!" );
    #endif
    
    ytfs_db = PQconnectdb( "host=database.stephencodes.com dbname=media_solution user=postgres password=yellowboots" );
    Py_Initialize();
    
    printf( "test" );
    
    return NULL;
}

void ytfs_destroy( void *args )
{
    Py_Finalize();
    PQfinish( ytfs_db );
    
    #ifdef DEBUG
    syslog( LOG_NOTICE, "YTFS terminated!" );
    closelog();
    #endif
}

int ytfs_create( const char *path, mode_t mode, struct fuse_file_info *fi )
{
    int retval = 0;
    int fd;
    char fpath[PATH_MAX];
    
    full_fs_path( fpath, path );
    fd = creat( fpath, mode );
    
    if( fd < 0 )
        retval = error( "ytfs_create" );
    
    fi->fh = fd;
    return retval;
}

int ytfs_fgetattr( const char *path, struct stat *statbuf, struct fuse_file_info *fi )
{
    int retval;
    
    if( !strcmp( path, "/" ) )
        return ytfs_getattr( path, statbuf );
    
    retval = fstat( fi->fh, statbuf );
    
    if( retval < 0 )
        retval = error( "ytfs_fgetattr" );
    
    return retval;
}

int ytfs_utimens( const char *path, const struct timespec tv[2] )
{
    int retval, checksum_success;
    char fpath[PATH_MAX], checksum[32], extension[PATH_MAX];
    
    checksum_success = get_checksum_and_extension( ytfs_db, checksum, extension, path );
	    
    if( checksum_success < 0 )
        return -ENOENT;
        
    full_fs_path( fpath, "/" );
    strcat( fpath, checksum );
    strcat( fpath, extension );
    
    retval = utimensat( 0, (const char *)full_fs_path, tv, 0 );
    
    if( retval < 0 )
        retval = error( "ytfs_utimens" );
    
    return retval;
}

int main( int argc, char *argv[] )
{
    int real_path_term;
    
    // Get the full path of the folder the binary is in
    realpath( argv[0], bin_path );
    real_path_term = last_index_of( bin_path, '/' );
    bin_path[real_path_term] = '\0';
    
    // Get the full path of the mount point
    realpath( argv[2], fs_path );
    
    return fuse_main( argc - 1, argv, &ytfs_oper, NULL );
}

