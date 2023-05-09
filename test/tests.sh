# @author    Clément Vidon
# @created   230320 00:00:00  by  clem@spectre
# @modified  230324 11:39:29  by  clem@spectre
# @filename  tests.sh

#!/bin/bash

r='\033[0;31m'
g='\033[0;32m'
b='\033[0;34m'
y='\033[0;33m'
c='\033[0;36m'
m='\033[0;35m'
x='\033[0m'

prompt_continue() {
    echo -en "${g}Please press enter to continue...${x}"
    read -r
}

run_tests() {
    $1
}

run_build() {
    local build_type="$1"
    local command="$2"
    local description="$3"

    make clean
    make "$build_type"
    clear
    echo -e "${g}Running tests on $description...${x}"
    run_tests "$command"
    prompt_continue
}

program="$1"
leaks="$2"

run_build "asan" "./$program" "a development build with Address Sanitizer"
run_build "all" "$leaks ./$program" "a production build with Valgrind"
make clean
