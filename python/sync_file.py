import sys
import boto3
import re
import urllib
import psycopg2
from tinytag import TinyTag

S3_BUCKET = 'media-solution'
S3_BASE_URL = 'https://s3.amazonaws.com/' + S3_BUCKET + '/'

file_path = sys.argv[1]

file_name_regex  = re.compile( r'\/?(\w*)\.(\w*)$' )
file_name_parsed = file_name_regex.search( file_path )

checksum         = file_name_parsed.group( 1 )
file_extension   = file_name_parsed.group( 2 )

s3_file_key      = checksum + '.' + file_extension
external_url     = S3_BASE_URL + s3_file_key

tag = TinyTag.get( file_path )

title        = tag.title
artist       = tag.artist
album        = tag.album
track_number = tag.track
bit_rate     = tag.bitrate
sample_rate  = tag.samplerate
duration     = tag.duration
year         = tag.year
file_size    = tag.filesize

s3  = boto3.client( 's3' )

try:
    s3.get_object( Bucket=S3_BUCKET, Key=s3_file_key )
except:
    s3.upload_file( file_path, S3_BUCKET, s3_file_key )

try:
    connection = psycopg2.connect( "dbname='media_solution' user='postgres' host='database.stephencodes.com' password='yellowboots'" )
except:
    raise Exception( "Unable to connect to the database" )

cursor = connection.cursor()

query = """SELECT fn_get_or_create_song
                  (
                    COALESCE( %s, 'Unknown' )::VARCHAR,
                    %s::INTEGER,
                    %s::NUMERIC,
                    %s::NUMERIC,
                    %s::INTEGER,
                    COALESCE( %s, 'Unknown' )::VARCHAR,
                    COALESCE( %s, 'Unknown' )::VARCHAR,
                    COALESCE( %s, 0)::INTEGER,
                    %s::VARCHAR,
                    %s::VARCHAR,
                    %s::VARCHAR,
                    %s::INTEGER,
                    %s::VARCHAR
                  )"""

params = [
    title,
    track_number,
    bit_rate,
    sample_rate,
    duration,
    artist,
    album,
    year,
    s3_file_key,
    file_extension,
    checksum,
    file_size,
    external_url
]

cursor.execute( query, params )

connection.commit()
