#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <libpq-fe.h>
#include <limits.h>

extern char bin_path[PATH_MAX];
extern char fs_path[PATH_MAX];
extern char *store_path;

inline int error( char *func );
int get_slash_count( const char *path );
int last_index_of( const char *path, char c );
int python( char *script_file, int argc, char *argv[] );
int get_checksum_and_extension( PGconn *ytfs_db, char buf[32], char extension[PATH_MAX], const char *path );
int get_file_size( PGconn *ytfs_db, const char *path );
inline void full_fs_path( char buf[PATH_MAX], const char *path );
inline void full_bin_path( char buf[PATH_MAX], const char *path );
inline void full_bin_python_path( char buf[PATH_MAX], char *py );
void md5( char *filename, char buf[32] );
inline int is_valid_file( const char *path );
inline int is_valid_folder( const char *path );

#endif // UTILITY_H_INCLUDED
