CREATE TABLE tb_file
(
    file            SERIAL PRIMARY KEY,
    name            VARCHAR(256) NOT NULL,
    size            INTEGER NOT NULL,
    extension       VARCHAR(64) NOT NULL,
    remote_url      VARCHAR(256) NOT NULL,
    checksum        VARCHAR(256) NOT NULL,
    last_accessed   TIMESTAMP NOT NULL DEFAULT now(),
    created         TIMESTAMP NOT NULL DEFAULT now(),
    UNIQUE( checksum )
);
COMMENT ON TABLE tb_file IS
'A file that is stored in S3.';

CREATE TABLE tb_artist
(
    artist SERIAL PRIMARY KEY,
    name   VARCHAR(256) NOT NULL
);
COMMENT ON TABLE tb_artist IS
'The artist of any number of albums';

CREATE TABLE tb_album
(
    album  SERIAL PRIMARY KEY,
    name   VARCHAR(256) NOT NULL,
    artist INTEGER REFERENCES tb_artist NOT NULL,
    year   INTEGER NOT NULL
);
COMMENT ON TABLE tb_album IS
'An album that contains songs.';

CREATE TABLE tb_song
(
    song         SERIAL PRIMARY KEY,
    name         VARCHAR(256) NOT NULL,
    track_number INTEGER,
    bit_rate     INTEGER,
    sample_rate  INTEGER,
    duration     INTEGER,
    album        INTEGER REFERENCES tb_album NOT NULL,
    file         INTEGER REFERENCES tb_file  NOT NULL
);
COMMENT ON TABLE tb_song IS
'A song that is contained in an album.';
