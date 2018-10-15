clang-tidy -p bin/compile_commands.json -config="" -header-filter="include/basilisk/*" src/*.cpp tests/*.cpp > clang-tidy.log
