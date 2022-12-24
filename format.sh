#!/bin/bash

cd "$(dirname "$(realpath "$0")")"

find lib -regex '.*\.[ch]pp' -exec clang-format -i {} \;
find gui -regex '.*\.[ch]pp' -exec clang-format -i {} \;
