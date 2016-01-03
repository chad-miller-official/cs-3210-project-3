ALTER SEQUENCE tb_album_album_seq RESTART WITH 1;
ALTER SEQUENCE tb_artist_artist_seq RESTART WITH 1;
ALTER SEQUENCE tb_file_file_seq RESTART WITH 1;
ALTER SEQUENCE tb_song_song_seq RESTART WITH 1;

INSERT INTO tb_artist (name)
     VALUES ('American Football');

INSERT INTO tb_album (name, artist, year)
     VALUES ('American Football', 1, 1999);

INSERT INTO tb_file (name, size, remote_url, checksum)
     VALUES ('1 - Never Meant.mp3', 0, '', 0),
            ('2 - The Summer Ends.mp3', 0, '', 0),
            ('3 - Honestly.mp3', 0, '', 0),
            ('4 - For Sure.mp3', 0, '', 0),
            ('5 - You Know I Should Be Leaving Soon.mp3', 0, '', 0),
            ('6 - But the Regrets Are Killing Me.mp3', 0, '', 0),
            ('7 - Ill See You When Were Both Not So Emotional.mp3', 0, '', 0),
            ('8 - Stay Home.mp3', 0, '', 0),
            ('9 - The One With the Wurlitzer.mp3', 0, '', 0);

INSERT INTO tb_song (name, album, file)
     VALUES ('Never Meant', 1, 1),
            ('The Summer Ends', 1, 2),
            ('Honestly?', 1, 3),
            ('For Sure', 1, 4),
            ('You Know I Should Be Leaving Soon', 1, 5),
            ('But the Regrets Are Killing Me', 1, 6),
            ('I''ll See You When We''re Both Not So Emotional', 1, 7),
            ('Stay Home', 1, 8),
            ('The One With the Wurlitzer', 1, 9);
