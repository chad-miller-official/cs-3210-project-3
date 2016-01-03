import os.path
import psycopg2
import sys
import urllib2
import urllib

file_system_root = sys.argv[1]  # This MUST end in a slash
path             = sys.argv[2]  # This MUST have at least two slashes

if not path.endswith( '/.hidden' ):
    S3_BUCKET   = 'media-solution'
    S3_BASE_URL = 'https://s3.amazonaws.com/' + S3_BUCKET + '/'

    try:
        connection = psycopg2.connect( "dbname='media_solution' user='postgres' host='database.stephencodes.com' password='yellowboots'" )
    except:
        raise Exception( 'Unable to connect to the database' )

    true_path_index = path.rfind( '/', 0, path.rfind( '/' ) )
    true_path       = path[true_path_index + 1:]
    cursor          = connection.cursor()
    query           = "SELECT f.checksum, \
                              '.' || f.extension \
                         FROM tb_file f \
                         JOIN tb_song s \
                           ON s.file = f.file \
                         JOIN tb_album a \
                           ON s.album = a.album \
                        WHERE a.name || '/' || s.track_number || ' - ' || s.name || '.' || f.extension = %s"
    params          = [ true_path ]

    try:
        cursor.execute( query, params )
    except:
        raise Exception( 'Unable to execute checksum query' )

    row       = cursor.fetchone()
    checksum  = row[0]
    extension = row[1]
    key_name  = checksum + extension
    full_path = file_system_root + key_name

    if not os.path.exists( full_path ):
        download_url = S3_BASE_URL + urllib.quote_plus( key_name )
        mp3file      = urllib2.urlopen( download_url )
        output       = open( full_path, 'wb' )
        output.write( mp3file.read() )
        output.close()

