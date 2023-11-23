### Easy to build, fast L1 test for sqlite backend

1. Install sqlite, ex. `sudo apt install -y libsqlite3-dev`
2. Install valgrind, coverage, ex. `sudo apt install -y valgrind lcov`
3. Build Thunder https://rdkcentral.github.io/Thunder/introduction/build_linux/ (use default branches and default instructions and cherry pick your changes)
4. Build and run tests
```
cmake -S rdkservices/PersistentStore/sqlite/l1test \
      -B build/sqlitel1test \
      -DCMAKE_INSTALL_PREFIX="install/usr" \
      -DCMAKE_CXX_FLAGS="--coverage -Wall -Werror"
cmake --build build/sqlitel1test --target install
PATH=${PWD}/install/usr/bin:${PATH} \
    LD_LIBRARY_PATH=${PWD}/install/usr/lib:${LD_LIBRARY_PATH} \
    valgrind \
    --tool=memcheck \
    --log-file=valgrind_log \
    --leak-check=yes \
    --show-reachable=yes \
    --track-fds=yes \
    --fair-sched=try \
    sqlitel1test
lcov -c -o coverage.info -d build/sqlitel1test
genhtml -o coverage coverage.info
```
