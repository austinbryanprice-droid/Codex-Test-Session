#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
output=$(mktemp /tmp/frontline-tests.XXXXXX)
trap 'rm -f "$output"' EXIT
"${CXX:-g++}" -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -g Tests/battle_tests.cpp -o "$output"
"$output"
