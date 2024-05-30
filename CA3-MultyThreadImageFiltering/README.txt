Operating System project 3 - serial and parallel image filtering using thread
Mohammad Sadegh Aghili

- first open new terminal tab at this folder
- navigate to ./serial or ./parallel folder. (cd serial or cd parallel)
- make each ./serial or ./parallel and then for both : ./ImageFilters.out "path/to/input/img" and see the result pics:
    - for serial: First_Serial.bmp and Second_Serial.bmp
    - for parallel: First_Parallel.bmp and Second_Parallel.bmp

- if you run each program you can see Execution Time.

- you can see ratio of speedup by running speedup.sh:
    - chmod +x speedup.sh
    - ./speedup.sh
        - it run each 5 time and then print the average for both parallel and serial and then calculate speedup!
