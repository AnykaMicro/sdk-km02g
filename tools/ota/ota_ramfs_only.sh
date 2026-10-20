#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
set -e

# Version configuration - Semantic Versioning
MAJOR_VERSION=0
MINOR_VERSION=0
PATCH_VERSION=0

# Compute numeric version for header storage
version=$((MAJOR_VERSION << 16 | MINOR_VERSION << 8 | PATCH_VERSION))
version_string="${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}"

# Output filename with version tag
output_package="update.bin"

# Timestamp used in package header
timestamp=$(date +%s)

# Define ONLY the required partitions to include
partitions=(
"flash11,ramfs.img,RAMFS,0x881000,0x24e000"
"flash13,usr.jffs2,CONFIG,0xe9e000,0x20000"
)

# Each partition entry is fixed at 44 bytes:
# 32 bytes name + 4 bytes size + 4 bytes offset + 4 bytes checksum
partition_info_size=$((44 * ${#partitions[@]}))

# Fixed-size header fields
header_size=90
partition_info_offset=$header_size
total_image_size=0
checksum_head=0

# Calculate total size of included images
# and refuse images that are missing or larger than their partition
for partition_info in "${partitions[@]}"; do
    IFS=',' read -r partition file_name partition_name start_address partition_size <<< "$partition_info"
    if [ ! -f "$file_name" ]; then
        echo "Error: $file_name not found. Build it first: make install" >&2
        exit 1
    fi
    file_size=$(stat -c%s "$file_name")
    if [ "$file_size" -gt $((partition_size)) ]; then
        echo "Error: $file_name is $file_size bytes, larger than the $partition_name partition ($((partition_size)) bytes)" >&2
        exit 1
    fi
    total_image_size=$((total_image_size + file_size))
done

# Total package size = header + partition table + image data
total_package_size=$((header_size + partition_info_size + total_image_size))
echo "Total package size: $total_package_size bytes"
: > "$output_package"

# Write package header to output file
{
    # Checksum placeholder (will update later)
    printf "%02x%02x%02x%02x" \
    $(( checksum_head & 0xff )) \
    $(( (checksum_head >> 8) & 0xff )) \
    $(( (checksum_head >> 16) & 0xff )) \
    $(( (checksum_head >> 24) & 0xff )) | xxd -r -p >> "$output_package"

    # Version number
    printf "%02x%02x%02x%02x" \
    $(( version & 0xff )) \
    $(( (version >> 8) & 0xff )) \
    $(( (version >> 16) & 0xff )) \
    $(( (version >> 24) & 0xff )) | xxd -r -p >> "$output_package"

    # Timestamp (8 bytes)
    printf "%02x%02x%02x%02x%02x%02x%02x%02x" \
    $(( timestamp & 0xff )) \
    $(( (timestamp >> 8) & 0xff )) \
    $(( (timestamp >> 16) & 0xff )) \
    $(( (timestamp >> 24) & 0xff )) \
    $(( (timestamp >> 32) & 0xff )) \
    $(( (timestamp >> 40) & 0xff )) \
    $(( (timestamp >> 48) & 0xff )) \
    $(( (timestamp >> 56) & 0xff )) | xxd -r -p >> "$output_package"

    # Total package size
    printf "%02x%02x%02x%02x" \
    $(( total_package_size & 0xff )) \
    $(( (total_package_size >> 8) & 0xff )) \
    $(( (total_package_size >> 16) & 0xff )) \
    $(( (total_package_size >> 24) & 0xff )) | xxd -r -p >> "$output_package"

    # Partition info offset
    printf "%02x%02x%02x%02x" \
    $(( partition_info_offset & 0xff )) \
    $(( (partition_info_offset >> 8) & 0xff )) \
    $(( (partition_info_offset >> 16) & 0xff )) \
    $(( (partition_info_offset >> 24) & 0xff )) | xxd -r -p >> "$output_package"

    # Size of partition info block
    printf "%02x%02x%02x%02x" \
    $(( partition_info_size & 0xff )) \
    $(( (partition_info_size >> 8) & 0xff )) \
    $(( (partition_info_size >> 16) & 0xff )) \
    $(( (partition_info_size >> 24) & 0xff )) | xxd -r -p >> "$output_package"

    # Reserved field (62 zeroed bytes)
    printf "%62s" "" | tr ' ' '\0' >> "$output_package"
}

# Write partition metadata entries
offset=$((header_size + partition_info_size)) # Start offset for image data
for partition_info in "${partitions[@]}"; do
    IFS=',' read -r partition file_name partition_name start_address partition_size <<< "$partition_info"
    partition_size=$(stat -c%s "$file_name")
    echo "Partition $file_name has size $partition_size"

    image_checksum=$(sum < "$file_name" | awk '{print $1}')

    {
        # Partition name padded/truncated to 32 bytes
        printf "%-32s" "$partition_name" | tr ' ' '\0' >> "$output_package"

        # Image size
        printf "%02x%02x%02x%02x" \
        $(( partition_size & 0xff )) \
        $(( (partition_size >> 8) & 0xff )) \
        $(( (partition_size >> 16) & 0xff )) \
        $(( (partition_size >> 24) & 0xff )) | xxd -r -p >> "$output_package"

        # Offset where image starts in package
        printf "%02x%02x%02x%02x" \
        $(( offset & 0xff )) \
        $(( (offset >> 8) & 0xff )) \
        $(( (offset >> 16) & 0xff )) \
        $(( (offset >> 24) & 0xff )) | xxd -r -p >> "$output_package"

        # Image checksum
        printf "%02x%02x%02x%02x" \
        $(( 10#$image_checksum & 0xff )) \
        $(( (10#$image_checksum >> 8) & 0xff )) \
        $(( (10#$image_checksum >> 16) & 0xff )) \
        $(( (10#$image_checksum >> 24) & 0xff )) | xxd -r -p >> "$output_package"
    }

    offset=$((offset + partition_size)) # Update next offset
done

# Append real image data only for selected partitions
for partition_info in "${partitions[@]}"; do
    IFS=',' read -r partition file_name partition_name start_address partition_size <<< "$partition_info"
    echo "Including $partition_name ($file_name) into package."
    cat "$file_name" >> "$output_package"
done

# Recalculate full-package checksum excluding first 4 bytes (checksum itself)
dd if="$output_package" bs=1 skip=4 of=part1.bin status=none
checksum_head=$(sum part1.bin | awk '{print $1}')

# Overwrite initial checksum field
{
   printf "%02x%02x%02x%02x" \
   $(( 10#$checksum_head & 0xff )) \
   $(( (10#$checksum_head >> 8) & 0xff )) \
   $(( (10#$checksum_head >> 16) & 0xff )) \
   $(( (10#$checksum_head >> 24) & 0xff )) | xxd -r -p
} | dd of="$output_package" bs=1 seek=0 conv=notrunc

echo "Checksum: $checksum_head written to $output_package"

echo ""
echo "=========================================="
echo "Upgrade package created successfully!"
echo "=========================================="
echo "Package file: $output_package"
echo "Version: $version_string (0x$(printf '%08x' $version))"
echo "Timestamp: $(date -d @$timestamp)"
echo "Total size: $total_package_size bytes"
echo "Included partitions: ${#partitions[@]}"
echo "=========================================="

# Cleanup temporary file
rm part1.bin
