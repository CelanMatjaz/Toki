format() {
    find $1 -iname *.h -o -iname *.cpp | xargs clang-format -i
}

format "toki_engine"
format "toki_sandbox"
