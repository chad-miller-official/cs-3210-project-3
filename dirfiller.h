#ifndef DIRFILLER_H_INCLUDED
#define DIRFILLER_H_INCLUDED

#include <fuse.h>
#include <libpq-fe.h>

void filler_root( void *buf, fuse_fill_dir_t filler );
void filler_all_albums( PGconn *ytfs_db, void *buf, fuse_fill_dir_t filler );
void filler_all_artists( PGconn *ytfs_db, void *buf, fuse_fill_dir_t filler );
void filler_all_decades( PGconn *ytfs_db, void *buf, fuse_fill_dir_t filler );
void filler_albums_from_artist( PGconn *ytfs_db, char *artist, void *buf, fuse_fill_dir_t filler );
void filler_albums_from_decade( PGconn *ytfs_db, char *decade, void *buf, fuse_fill_dir_t filler );
void filler_tracks_in_album( PGconn *ytfs_db, char *album, void *buf, fuse_fill_dir_t filler );

#endif // DIRFILLER_H_INCLUDED

