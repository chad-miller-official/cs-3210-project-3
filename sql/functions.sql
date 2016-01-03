CREATE OR REPLACE FUNCTION fn_get_or_create_song
(
    in_title          VARCHAR,
    in_track_number   INTEGER,
    in_bit_rate       NUMERIC,
    in_sample_rate    NUMERIC,
    in_duration       INTEGER,
    in_artist         VARCHAR,
    in_album          VARCHAR,
    in_year           INTEGER,
    in_file_name      VARCHAR,
    in_file_extension VARCHAR,
    in_checksum       VARCHAR,
    in_file_size      INTEGER,
    in_remote_url     VARCHAR
)
 RETURNS INTEGER
AS $_$
DECLARE
    my_pk_artist INTEGER;
    my_pk_album  INTEGER;
    my_pk_song   INTEGER;
    my_pk_file   INTEGER;
BEGIN
    SELECT artist
      INTO my_pk_artist
      FROM tb_artist
     WHERE name = in_artist;

    IF( my_pk_artist IS NULL ) THEN
        INSERT INTO tb_artist
        (
            name
        )
        VALUES
        (
            in_artist
        )
        RETURNING artist
        INTO my_pk_artist;
    END IF;

    SELECT album
      INTO my_pk_album
      FROM tb_album
     WHERE name = in_album
       AND artist = my_pk_artist;

    IF( my_pk_album IS NULL ) THEN
        INSERT INTO tb_album
        (
            name,
            artist,
            year
        )
        VALUES
        (
            in_album,
            my_pk_artist,
            in_year
        )
        RETURNING album
        INTO my_pk_album;
    END IF;

    SELECT file
      INTO my_pk_file
      FROM tb_file
     WHERE checksum = in_checksum;

    IF( my_pk_file IS NULL ) THEN
        INSERT INTO tb_file
        (
            name,
            size,
            extension,
            remote_url,
            checksum
        )
        VALUES
        (
            in_file_name,
            in_file_size,
            in_file_extension,
            in_remote_url,
            in_checksum
        )
        RETURNING file
        INTO my_pk_file;
    END IF;

    SELECT song
      INTO my_pk_song
      FROM tb_song
     WHERE file = my_pk_file;

    IF( my_pk_song IS NULL ) THEN
        INSERT INTO tb_song
        (
            name,
            track_number,
            bit_rate,
            sample_rate,
            duration,
            file,
            album
        )
        VALUES
        (
            in_title,
            in_track_number,
            in_bit_rate,
            in_sample_rate,
            in_duration,
            my_pk_file,
            my_pk_album
        )
        RETURNING song
        INTO my_pk_song;
    END IF;

    RETURN my_pk_song;
END
 $_$
 	LANGUAGE 'plpgsql';
COMMENT ON FUNCTION fn_get_or_create_song
(
    VARCHAR,
    INTEGER,
    NUMERIC,
    NUMERIC,
    INTEGER,
    VARCHAR,
    VARCHAR,
    INTEGER,
    VARCHAR,
    VARCHAR,
    VARCHAR,
    INTEGER,
    VARCHAR
)
IS 'Creates or retreives all necessary data to insert a song into the DB';

CREATE OR REPLACE FUNCTION fn_delete_album_if_empty
(
    in_pk_album INTEGER
)
 RETURNS INTEGER
AS $_$
DECLARE
    my_song_count INTEGER;
BEGIN
    SELECT count(*)
      INTO my_song_count
      FROM tb_song
     WHERE album = in_pk_album;

    IF( my_song_count = 0 ) THEN
        DELETE FROM tb_album WHERE album = in_pk_album;
    END IF;

    RETURN NULL;
END
 $_$
 	LANGUAGE 'plpgsql';
COMMENT ON FUNCTION fn_delete_album_if_empty( INTEGER ) IS
'Delete an album from the database if it is empty';

CREATE OR REPLACE FUNCTION fn_delete_artist_with_no_albums
(
    in_pk_artist INTEGER
)
 RETURNS INTEGER
AS $_$
DECLARE
    my_album_count INTEGER;
BEGIN
    SELECT count(*)
      INTO my_album_count
      FROM tb_album
     WHERE artist = in_pk_artist;

    IF( my_album_count = 0 ) THEN
        DELETE FROM tb_artist WHERE artist = in_pk_artist;
    END IF;

    RETURN NULL;
END
 $_$
 	LANGUAGE 'plpgsql';
COMMENT ON FUNCTION fn_delete_artist_with_no_albums( INTEGER ) IS
'Delete an artist if they have no albums left';

CREATE OR REPLACE FUNCTION fn_delete_song_by_checksum
(
    in_checksum VARCHAR
)
 RETURNS INTEGER
AS $_$
DECLARE
    my_song  INTEGER;
    my_album INTEGER;
    my_file  INTEGER;
    my_artist INTEGER;
BEGIN
    SELECT s.song,
           s.album,
           s.file,
           ar.artist
      INTO my_song,
           my_album,
           my_file,
           my_artist
      FROM tb_song s
      JOIN tb_album al
        ON al.album = s.album
      JOIN tb_artist ar
        ON al.artist = ar.artist
      JOIN tb_file f
        ON f.file = s.file
     WHERE f.checksum = in_checksum;

    DELETE FROM tb_song WHERE song = my_song;

    DELETE FROM tb_file WHERE file = my_file;

    PERFORM fn_delete_album_if_empty( my_album );

    PERFORM fn_delete_artist_with_no_albums( my_artist );

    RETURN NULL;
END
 $_$
 	LANGUAGE 'plpgsql';
COMMENT ON FUNCTION fn_delete_song_by_checksum( VARCHAR ) IS
'Delete a song and the corresponding data';
