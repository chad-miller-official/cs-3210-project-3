#!/bin/bash

PSQL_STRING='psql -h database.stephencodes.com -U postgres'

$PSQL_STRING -d postgres -f kill_connections.sql -1

$PSQL_STRING -d postgres -c 'DROP DATABASE media_solution'
$PSQL_STRING -d postgres -c 'CREATE DATABASE media_solution'

$PSQL_STRING -d media_solution -f create.sql -1
$PSQL_STRING -d media_solution -f functions.sql -1
