#!/bin/bash
# screen_transfer.sh — FPGA initialization and desktop streaming
# Usage: sudo ./screen_transfer.sh [options]
#
# Options:
#   -c  channel number (0-7, default 0)
#   -W  width (default 1920)
#   -H  height (default 1080)
#   -F  frame rate (default 30)
#   -C  card number (default 0)
#   -D  display (default :1)
#   -X  path to XAUTHORITY (default /run/user/1000/gdm/Xauthority)
#   -h  help

set -e

# ---------------------------------------------------------------
# Default values
# ---------------------------------------------------------------
CHANNEL=0
CARD=0
WIDTH=1920
HEIGHT=1080
FPS=30
DISP=":0"
XAUTH="/run/user/1000/gdm/Xauthority"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# tools/ in the same directory or one level up
if [[ -d "$SCRIPT_DIR/tools" ]]; then
    TOOLS_DIR="$SCRIPT_DIR/tools"
elif [[ -d "$SCRIPT_DIR/../tools" ]]; then
    TOOLS_DIR="$(cd "$SCRIPT_DIR/../tools" && pwd)"
else
    echo "[ERROR] tools/ directory not found"
    exit 1
fi

REG_RW="$TOOLS_DIR/reg_rw"
DMA_TOOL="$TOOLS_DIR/dma_to_device_video"
XDMA_USER="/dev/xdma0_user"
XDMA_H2C="/dev/xdma0_h2c_0"

# ---------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------
usage() {
    echo "Usage: sudo $0 [options]"
    echo "  -c  channel 0-7          (default: $CHANNEL)"
    echo "  -W  width                (default: $WIDTH)"
    echo "  -H  height               (default: $HEIGHT)"
    echo "  -F  fps                  (default: $FPS)"
    echo "  -C  card number          (default: $CARD)"
    echo "  -D  display              (default: $DISP)"
    echo "  -X  path to XAUTHORITY   (default: $XAUTH)"
    echo ""
    echo "Example:"
    echo "  sudo $0 -D :1 -X /run/user/1000/gdm/Xauthority"
    exit 0
}

while getopts "c:W:H:F:C:D:X:h" opt; do
    case $opt in
        c) CHANNEL="$OPTARG" ;;
        W) WIDTH="$OPTARG" ;;
        H) HEIGHT="$OPTARG" ;;
        F) FPS="$OPTARG" ;;
        C) CARD="$OPTARG" ;;
        D) DISP="$OPTARG" ;;
        X) XAUTH="$OPTARG" ;;
        h) usage ;;
        *) usage ;;
    esac
done

# ---------------------------------------------------------------
# Checks
# ---------------------------------------------------------------
if [[ $EUID -ne 0 ]]; then
    echo "[ERROR] Run as root: sudo $0"
    exit 1
fi

for f in "$REG_RW" "$DMA_TOOL"; do
    if [[ ! -x "$f" ]]; then
        echo "[ERROR] $f not found — run make in tools/"
        exit 1
    fi
done

for dev in "$XDMA_USER" "$XDMA_H2C"; do
    if [[ ! -c "$dev" ]]; then
        echo "[ERROR] Device $dev not found — load the driver"
        exit 1
    fi
done

FRAME_SIZE=$((WIDTH * HEIGHT * 2))
FPS_VALUE=$((200000000 / FPS))

echo "[*] Channel=$CHANNEL Card=$CARD Resolution=${WIDTH}x${HEIGHT} FPS=$FPS"
echo "[*] DISPLAY=$DISP XAUTHORITY=$XAUTH"

# ---------------------------------------------------------------
# FPGA Initialization (from transfer_run.sh)
# ---------------------------------------------------------------
echo ""
echo "[1/5] FPGA reset..."
$REG_RW $XDMA_USER 0x30004 w 0xff
sleep 1
$REG_RW $XDMA_USER 0x30004 w 0x0
$REG_RW $XDMA_USER 0x30008 w 0x0

for addr in 0x70000 0x80000 0x90000 0xa0000 0xb0000 0xc0000 0xd0000 0xe0000; do
    $REG_RW $XDMA_USER $addr w 0x2
done

echo "[2/5] Setting resolution for channel $CHANNEL..."
case $CHANNEL in
    0) $REG_RW $XDMA_USER 0x30100 w $WIDTH;  $REG_RW $XDMA_USER 0x30104 w $HEIGHT
       $REG_RW $XDMA_USER 0x301b0 w $((WIDTH/2)); $REG_RW $XDMA_USER 0x70040 w $HEIGHT ;;
    1) $REG_RW $XDMA_USER 0x30108 w $WIDTH;  $REG_RW $XDMA_USER 0x3010c w $HEIGHT
       $REG_RW $XDMA_USER 0x301b4 w $((WIDTH/2)); $REG_RW $XDMA_USER 0x80040 w $HEIGHT ;;
    2) $REG_RW $XDMA_USER 0x30110 w $WIDTH;  $REG_RW $XDMA_USER 0x30114 w $HEIGHT
       $REG_RW $XDMA_USER 0x301b8 w $((WIDTH/2)); $REG_RW $XDMA_USER 0x90040 w $HEIGHT ;;
    3) $REG_RW $XDMA_USER 0x30118 w $WIDTH;  $REG_RW $XDMA_USER 0x3011c w $HEIGHT
       $REG_RW $XDMA_USER 0x301bc w $((WIDTH/2)); $REG_RW $XDMA_USER 0xa0040 w $HEIGHT ;;
    4) $REG_RW $XDMA_USER 0x30120 w $WIDTH;  $REG_RW $XDMA_USER 0x30124 w $HEIGHT
       $REG_RW $XDMA_USER 0x301c0 w $((WIDTH/2)); $REG_RW $XDMA_USER 0xb0040 w $HEIGHT ;;
    5) $REG_RW $XDMA_USER 0x30128 w $WIDTH;  $REG_RW $XDMA_USER 0x3012c w $HEIGHT
       $REG_RW $XDMA_USER 0x301c4 w $((WIDTH/2)); $REG_RW $XDMA_USER 0xc0040 w $HEIGHT ;;
    6) $REG_RW $XDMA_USER 0x30130 w $WIDTH;  $REG_RW $XDMA_USER 0x30134 w $HEIGHT
       $REG_RW $XDMA_USER 0x301c8 w $((WIDTH/2)); $REG_RW $XDMA_USER 0xd0040 w $HEIGHT ;;
    7) $REG_RW $XDMA_USER 0x30138 w $WIDTH;  $REG_RW $XDMA_USER 0x3013c w $HEIGHT
       $REG_RW $XDMA_USER 0x301cc w $((WIDTH/2)); $REG_RW $XDMA_USER 0xe0040 w $HEIGHT ;;
esac

for addr in 0x30190 0x30194 0x30198 0x3019c 0x301a0 0x301a4 0x301a8 0x301ac; do
    $REG_RW $XDMA_USER $addr w 1100
done

echo "[3/5] Setting FPS ($FPS fps)..."
for addr in 0x30024 0x30028 0x3002c 0x30030 0x30034 0x30038 0x3003c 0x30040; do
    $REG_RW $XDMA_USER $addr w $FPS_VALUE
done
sleep 1

echo "[4/5] Final configuration..."
for addr in 0x70000 0x80000 0x90000 0xa0000 0xb0000 0xc0000 0xd0000 0xe0000; do
    $REG_RW $XDMA_USER $addr w 1
done
$REG_RW $XDMA_USER 0x3000c w 0xff

for addr in 0x30044 0x30048 0x3004c 0x30050 0x30054 0x30058 0x3005c 0x30060; do
    $REG_RW $XDMA_USER $addr w 100
done
$REG_RW $XDMA_USER 0x30020 w 0x0

echo "[5/5] Trigger activation..."
$REG_RW $XDMA_USER 0x30018 w 0x0
$REG_RW $XDMA_USER 0x30020 w 0x22222222
$REG_RW $XDMA_USER 0x30018 w 0xff

# ---------------------------------------------------------------
# Start screen capture
# ---------------------------------------------------------------
echo ""
echo "=================================================="
echo " FPGA is ready. Starting screen streaming."
echo " Stop with: Ctrl+C"
echo "=================================================="
echo ""

export DISPLAY="$DISP"
export XAUTHORITY="$XAUTH"

exec "$DMA_TOOL" \
    -C "$CARD" \
    -d "$XDMA_H2C" \
    -c "$CHANNEL" \
    -t Screen \
    -W "$WIDTH" \
    -H "$HEIGHT" \
    -F "$FPS" \
    -r 1000000000 \
    -s "$FRAME_SIZE"
