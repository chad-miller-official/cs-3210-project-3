SELECT pg_terminate_backend( pid )
  FROM pg_stat_activity
 WHERE datname = 'media_solution'
   AND pid != pg_backend_pid();
