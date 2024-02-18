#!/bin/bash
# Create .dmg file for macOS

# Adapt these variables to your needs
APP_VERS="v0.1.2"
DMG_NAME="Wakuseibokan_${APP_VERS}_macos"
OUTPUT_DMG_DIR="packages/"
APP_FILE="waku"

#otool -L waku
install_name_tool -change libstk-4.6.1.dylib @loader_path/lib/libstk-4.6.1.dylib "${APP_FILE}"

# The directory of the script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The temp directory used, within $DIR
WORK_DIR=`mktemp -d "${DIR}/tmp"`

# Check if tmp dir was created
if [[ ! "${WORK_DIR}" || ! -d "${WORK_DIR}" ]]; then
    echo "Could not create temp dir"
    exit 1
fi

# Function to deletes the temp directory
function cleanup {
    rm -rf "${WORK_DIR}"
    #echo "Deleted temp working directory ${WORK_DIR}"
}

# Register the cleanup function to be called on the EXIT signal
trap cleanup EXIT




# Copy application on temp dir
cp -R "${APP_FILE}" "${WORK_DIR}"
# Copy other files without hidden files
rsync -a --exclude=".*" "sounds" "${WORK_DIR}"
rsync -a --exclude=".*" "units"  "${WORK_DIR}"
rsync -a --exclude=".*" "terrain" "${WORK_DIR}"
rsync -a --exclude=".*" "structures" "${WORK_DIR}"
rsync -a --exclude=".*" "sky" "${WORK_DIR}"
rsync -a --exclude=".*" "water" "${WORK_DIR}"
rsync -a --exclude=".*" "system/mac/libstk-4.6.1.dylib" "${WORK_DIR}/lib/"
rsync -a --exclude=".*" "conf" "${WORK_DIR}"
rsync -a --exclude=".*" "savegames" "${WORK_DIR}"

# Create .dmg
hdiutil create -volname "${DMG_NAME}" -srcfolder "${WORK_DIR}" -ov -format UDZO "${OUTPUT_DMG_DIR}/${DMG_NAME}.dmg"
