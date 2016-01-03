# Prerequisites

### Python Packages
- `pip install boto3`    - Amazon AWS SDK for S3 File Uploads
- `pip install eyed3`    - Library for reading MP3 Metadata
- `pip install psycopg2` - PostgreSQL library

### AWS Configuration
- Create the file `~/.aws/credentials` with the following content:

    ```
    [default]
    aws_access_key_id = AKIAIKD7CRPQANK2YMBQ
    aws_secret_access_key = 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
    ```
- Create the file `~/.aws/config` with the following content:

    ```
    [default]
    region=us-east-1
    ```

### Running `sync_file.py`

Pass the script the absolute path of an MP3 file

`python sync_file.py messy_life.mp3`

### Running `download_file.py`

Pass the script the file system root directory, the artist name, the album name, and the track number.

`python download_file.py '/tmp/' "Cap'n Jazz" 'Analphabetapolothology' 2`
