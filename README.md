# YourTunes File System

### Requirements

* PostgreSQL 9.3
* PostgreSQL libpq
* Python 2.7
* A stable, persistent internet connection

### How To Mount

1. Run `make`.
2. Run `./ytfs.sh`.

### How To Unmount

1. Run `make clean`.

### Operations Supported

* `cp` to and from the file system
* `rm` from the file system
* `mv` to the file system

### Filetypes Supported

* MP3
* FLAC
* WAV
* OGG

### Common Problems

Occasionally, upon mounting, some or all directories will not be visible. To fix this, unmount and re-mount the system.
