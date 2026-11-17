#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
bin=$(mktemp)
trap 'rm -f "$bin"' EXIT
${CXX:-g++} -std=c++11 -Wall -Wextra -Werror -pedantic -I mock -I ../src test_scroll.cpp -o "$bin"
"$bin"
