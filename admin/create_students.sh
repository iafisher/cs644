#!/bin/bash

set -eu
source "/home/ian/.ian/foundation/shell/bash_functions.sh"

main() {
  ian_parse_flags "STUDENTS_FILE" "$@"
  p="${__args[STUDENTS_FILE]}"

  if [[ "$USER" != root ]]; then
    fatal "this command must be run as root"
  fi

  mapfile -t students < <(cut -d' ' -f 1 "$p" | tr '[:upper:]' '[:lower:]' | sort)

  duplicates=$(echo "${students[@]}" | xargs -n1 | uniq -d)
  if [[ -n "$duplicates" ]]; then
    fatal "duplicate student names:\n\n$duplicates"
  fi

  shared=/home/shared
  mkdir -p "$shared"
  chown ian:ian "$shared"

  group="students"
  for student in "${students[@]}"; do
    if [[ -e "/home/$student" ]]; then
      status "skipping account: $student (already exists)"
      continue
    fi

    status "creating account: $student"
    adduser "$student" --disabled-password --comment ""
    usermod -aG "$group" "$student"
    echo -e '\n\nsource "/usr/share/cs644/code/admin/shellrc"' >> "/home/$student/.bashrc"
    student_shared="$shared/$student"
    mkdir -p "$student_shared"
    chown "$student:$group" "$student_shared"
  done

  chmod 775 "$shared/"*

  echo
  status "created ${#students[@]} students"
}

main "$@"
