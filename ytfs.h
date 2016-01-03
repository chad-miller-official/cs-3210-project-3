#ifndef YTFS_H_INCLUDED
#define YTFS_H_INCLUDED

#include <fuse.h>

int ytfs_getattr( const char *path, struct stat *stbuf );
int ytfs_unlink( const char *path );
int ytfs_truncate( const char *path, off_t newsize );
int ytfs_open( const char *path, struct fuse_file_info *fi );
int ytfs_read( const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi );
int ytfs_write( const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi );
int ytfs_release( const char *path, struct fuse_file_info *fi );
int ytfs_readdir( const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi );
void* ytfs_init( struct fuse_conn_info *conn );
void ytfs_destroy( void *args );
int ytfs_create( const char *path, mode_t mode, struct fuse_file_info *fi );
int ytfs_fgetattr( const char *path, struct stat *statbuf, struct fuse_file_info *fi );
int ytfs_utimens( const char *path, const struct timespec tv[2] );

#endif // YTFS_H_INCLUDED

