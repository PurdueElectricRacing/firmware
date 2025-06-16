#!/usr/bin/env bash
#
# format.sh
#   ./format.sh format   # format all files in place
#   ./format.sh check    # validate formatting

SOURCE_DIRS=(
  common/amk
  common/bootloader
  common/common_defs
  common/faults
  common/log
  common/phal
  common/phal_F4_F7
  common/phal_G4
  common/phal_L4
  common/psched
  common/queue
  common/syscalls
  source
)

FILE_GLOB='\( -iname "*.c" -o -iname "*.h" \)'

format() {
  local target_dirs=()

  # If an argument was passed to `format`, use it as the only directory
  if [[ -n "$2" ]]; then
    if [[ -d "$2" ]]; then
      target_dirs=("$2")
    else
      echo "❌ Error: '$2' is not a directory."
      exit 1
    fi
  else
    target_dirs=("${SOURCE_DIRS[@]}")
  fi

  echo "Formatting files in: ${target_dirs[*]}"
  find "${target_dirs[@]}" -type f \( -iname '*.c' -o -iname '*.h' \) -exec clang-format -i --style=file {} +
  echo "✅ Formatting complete."
}

check() {
  local files=()
  local failed=0

  # Collect .c and .h files
  while IFS= read -r file; do
    files+=("$file")
  done < <(find "${SOURCE_DIRS[@]}" -type f \( -name '*.c' -o -name '*.h' \))

  echo "Checking formatting on ${#files[@]} files..."

  for f in "${files[@]}"; do
    if ! clang-format --dry-run --Werror --style=file "$f" >/dev/null; then
      echo "❌ Formatting error in: $f"
      failed=1
    fi
  done

  if [[ $failed -ne 0 ]]; then
    echo -e "\n❌ Some files are not properly formatted."
    echo "   Run: ./format.sh format"
    exit 1
  fi

  echo "✅ All files are properly formatted."
}

case "$1" in
  format) format "$@" ;;
  check)  check  "$@" ;;
  *)      echo "Usage: $0 {format|check}" && exit 2 ;;
esac
