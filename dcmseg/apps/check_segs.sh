#!/bin/bash

set -e

# Option for dump comparison
COMPARE_DUMP=false

# Argument parsing
FILES=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d)
            COMPARE_DUMP=true
            shift
            ;;
        *)
            FILES+=("$1")
            shift
            ;;
    esac
done

if [ ${#FILES[@]} -eq 0 ]; then
    echo "Usage: $0 [-d] <dicom-files-or-pattern>"
    echo "Example: $0 -d /path/to/files/*.dcm"
    exit 1
fi

OUTPUT_DIR="output"
mkdir -p "$OUTPUT_DIR"

echo "========== Pass 1: With segdigest =========="

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        base=$(basename "$file")
        converted_file="/tmp/converted_${base}"
        decompressed_file="/tmp/decompressed_${base}"
        final_file="/tmp/final_${base}"
        dump_file="/tmp/dump_${base}.txt"

        echo "Processing (with segdigest): $file"

        ./bin/segdigest "$file" "$converted_file"
        dcmdrle "$converted_file" "$decompressed_file"
        dcmconv +e "$decompressed_file" "$final_file"

        if $COMPARE_DUMP; then
            dcmdump "$final_file" > "$dump_file"
        else
            dcmdump "$final_file" +W "$OUTPUT_DIR"
        fi
    fi
done

echo "========== Pass 2: Without segdigest =========="

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        base=$(basename "$file")
        orig_decompressed_file="/tmp/orig_decompressed_${base}"
        orig_final_file="/tmp/orig_final_${base}"
        orig_dump_file="/tmp/orig_dump_${base}.txt"

        echo "Processing (without segdigest): $file"

        dcmdrle "$file" "$orig_decompressed_file"
        dcmconv +e "$orig_decompressed_file" "$orig_final_file"

        if $COMPARE_DUMP; then
            dcmdump "$orig_final_file" > "$orig_dump_file"
        else
            dcmdump "$orig_final_file" +W "$OUTPUT_DIR"
        fi
    fi
done

echo "========== Comparison =========="

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        base=$(basename "$file")
        final_file="/tmp/final_${base}"
        orig_final_file="/tmp/orig_final_${base}"

        echo "Comparing $final_file <-> $orig_final_file ..."

        if cmp -s "$final_file" "$orig_final_file"; then
            echo "→ Binary files: identical"
        else
            echo "→ Binary files: DIFFERENT"
            echo "  Byte differences (first 10):"
            cmp -l "$final_file" "$orig_final_file" | head -n 10
        fi

        if $COMPARE_DUMP; then
            dump_file="/tmp/dump_${base}.txt"
            orig_dump_file="/tmp/orig_dump_${base}.txt"
            echo "→ Dump comparison:"
            if diff -q "$dump_file" "$orig_dump_file" > /dev/null; then
                echo "  Text dumps: identical"
            else
                echo "  Text dumps: DIFFERENT (first few lines):"
                diff -u "$dump_file" "$orig_dump_file" | head -n 20
            fi
        fi

        echo
    fi
done
