#!/bin/bash

set -e
set -u

# 检测网络连通性
check_network() {
    local target_ip="$1"
    if ping -c 1 -W 1 "$target_ip" >/dev/null 2>&1; then
        echo "Network to $target_ip is reachable"
        return 0
    else
        echo "Network to $target_ip is NOT reachable, skip upload"
        return 1
    fi
}

TIMESTAMP=$(date +%Y%m%d%H%M%S)

TOP_DIR="$(cd "$(dirname "$0")" && pwd)"

MODULE_DIRS=(
	"7-new_sensor"
    "3-iomanager"
)

# 处理clean参数
if [ "${1:-}" = "clean" ]; then
    echo "Cleaning build directories..."
    rm -rf "${TOP_DIR}/nx_driver" "${TOP_DIR}/nx_dbg_driver"
    for dir in "${MODULE_DIRS[@]}"; do
        if [ -d "${TOP_DIR}/${dir}" ]; then
            echo "Cleaning ${dir}"
            make -C "${TOP_DIR}/${dir}" clean >/dev/null 2>&1 || true
        fi
    done
    echo "Clean completed"
    exit 0
fi

# 设置输出目录
if [ "${1:-}" = "debug" ]; then
    OUT_DIR="${TOP_DIR}/nx_dbg_driver"
    ZIP_FILE="${TOP_DIR}/nx_dbg_driver_${TIMESTAMP}.zip"
    echo "Building debug version, output to nx_dbg_driver/"
else
    OUT_DIR="${TOP_DIR}/nx_driver"
    ZIP_FILE="${TOP_DIR}/nx_driver_${TIMESTAMP}.zip"
fi

# 清理输出目录，确保重新构建
rm -rf "${OUT_DIR}"
mkdir -p "${OUT_DIR}"

# 定义构建函数
build_module() {
    local dir="$1"
    local debug="$2"  # "debug" 或 "release"
    local extra_args=""
    local suffix=""
    
    if [ "$debug" = "debug" ]; then
        extra_args="USR_DEBUG_ENABLE=1"
        suffix="_dbg"
    fi
    
    MOD_PATH="${TOP_DIR}/${dir}"
    
    echo
    echo ">>> Enter ${dir} (${debug} build)"
    
    if [ ! -d "${MOD_PATH}" ]; then
        echo "❌ ${dir} not found, skip"
        return 1
    fi
    
    cd "${MOD_PATH}"
    
    if [ -f "Makefile" ]; then
        echo "→ run make ${extra_args} in ${dir}"
        make clean >/dev/null 2>&1 || true
        make ${extra_args}
    else
        echo "⚠️  no Makefile in ${dir}, skip build"
        return 1
    fi
    
    # 收集 .ko 文件
    KO_FILES=$(find . -maxdepth 1 -name "*.ko")
    if [ -n "${KO_FILES}" ]; then
        echo "→ collect .ko from ${dir}"
        for ko in ${KO_FILES}; do
            # 重命名文件，如果是debug构建且文件名不包含_dbg，则添加后缀
            local base_ko=$(basename "$ko")
            if [ "$debug" = "debug" ] && [[ ! "$base_ko" =~ _dbg\.ko$ ]]; then
                local new_name="${base_ko%.ko}${suffix}.ko"
                cp -v "$ko" "${OUT_DIR}/${new_name}"
            else
                cp -v "$ko" "${OUT_DIR}/"
            fi
        done
    else
        echo "⚠️  no .ko generated in ${dir}"
    fi
    
    cd "${TOP_DIR}"
}

# 检查是否有debug参数
BUILD_DEBUG=false
if [ "${1:-}" = "debug" ]; then
    BUILD_DEBUG=true
    echo "Building debug version only"
else
    echo "Building release version only"
fi

# 构建release版本
if [ "$BUILD_DEBUG" = false ]; then
    for dir in "${MODULE_DIRS[@]}"; do
        build_module "$dir" "release"
    done
fi

# 构建debug版本
if [ "$BUILD_DEBUG" = true ]; then
    for dir in "${MODULE_DIRS[@]}"; do
        build_module "$dir" "debug"
    done
fi

cd "${TOP_DIR}"

cp -f ./pr_drive_version.sh "${OUT_DIR}"/

if [ -d "${OUT_DIR}" ] && [ -n "$(ls -A "${OUT_DIR}")" ]; then
    echo
    echo "📦 Compressing nx_driver -> ${ZIP_FILE}"
    rm -f "${ZIP_FILE}"
    cd "${OUT_DIR}"
    zip -r "${ZIP_FILE}" .
    cd "${TOP_DIR}"
    echo "✅ zip generated: ${ZIP_FILE}"
else
    echo "❌ nx_driver directory empty, skip zip"
    exit 1
fi

echo "===== Deploy Info ====="
echo "TARGET_IP  = $TARGET_IP"
echo "USER       = $USR"
echo "PASSWORD   = $PASSWORD"
echo "======================="

# 上传前检查网络连接
if check_network "$TARGET_IP"; then
    echo "Uploading ${ZIP_FILE} to ${USR}@${TARGET_IP}:/home/${USR}/nfs"
    sshpass -p "$PASSWORD" \
        scp "${ZIP_FILE}" "$USR@$TARGET_IP:/home/$USR/nfs"
    
    if [ -f "./7-new_sensor/to.sh" ]; then
        sshpass -p "$PASSWORD" \
            scp "./7-new_sensor/to.sh" "$USR@$TARGET_IP:/home/$USR/nfs"
    fi
else
    echo "Skipping upload due to network unreachable"
fi

echo "Build completed successfully"
