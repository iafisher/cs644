#!/bin/bash

set -eu
source "$IAN_BASH_PRELUDE"

main() {
  ian_parse_flags "STUDENTS_FILE" "$@"
  p="${__args[STUDENTS_FILE]}"

  local d="$(dirname "$(realpath "$0")")"
  mapfile -t students < <(cut -d' ' -f 1 "$p" | tr '[:upper:]' '[:lower:]' | sort)
  for student in "${students[@]}"; do
    local password="$(python3 "$d/mkpwd.py")"
    echo "$student: $password"
    echo "$student:$password" | sudo chpasswd
  done
}

main "$@"
