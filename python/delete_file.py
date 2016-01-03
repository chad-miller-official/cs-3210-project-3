import sys
import psycopg2

checksum = sys.argv[1]

try:
    connection = psycopg2.connect( "dbname='media_solution' user='postgres' host='database.stephencodes.com' password='yellowboots'" )
except:
    raise Exception( "Unable to connect to the database" )

cursor = connection.cursor()

query = "SELECT fn_delete_song_by_checksum( %s )"

params = [checksum]

try:
    cursor.execute( query, params )
except:
    raise

connection.commit()
