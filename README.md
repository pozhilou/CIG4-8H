
**FFmpeg SDK Compilation and Usage Guide (CIG4-8H)**

The video reinjection card adopts PCIe Gen3 technology, which can achieve 8-channel video/image injection and uses high-precision multi-channel time synchronization technology to ensure that multi-channel data can be output synchronously. The video reinjection card is a product specifically designed to address local video injection and simulation verification. Its core value lies in accurately reproducing real scene data streams, accelerating product testing and algorithm iteration, and achieving one-time data collection and repeated algorithm verification.

---

#### **1. Install Dependency Tools**
```bash
sudo apt-get update
sudo apt-get install openssh-server libtool git vim sshpass
```

---

#### **2. FFmpeg Environment Setup**
```bash
# Extract FFmpeg source package
tar -xf ffmpeg-4.4.4.tar.xz

# Install compilation dependencies
sudo apt-get install libsdl2-dev libsdl1.2-dev x264 libx264-dev

# Compile and install in extracted directory
cd ffmpeg-4.4.4
sudo ./configure --prefix=/usr/local/ffmpeg \
    --enable-shared --disable-static --disable-doc \
    --enable-ffmpeg --enable-ffplay --enable-ffprobe \
    --enable-gpl --enable-libx264 --enable-nonfree --disable-x86asm

# Compile and install (48-thread acceleration)
make -j48
sudo make install

# Configure environment variables
echo 'export PATH="$PATH:/usr/local/ffmpeg/bin"' >> ~/.bashrc
echo 'export CPATH="$CPATH:/usr/local/ffmpeg/include"' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/ffmpeg/lib"' >> ~/.bashrc

# Apply configuration
source ~/.bashrc
sudo ldconfig
```

---

#### **3. SDK Code Compilation**
```bash
# Compile xdma driver
cd xdma
make

# Compile toolchain
cd ../tools
make clean
make
```

---

#### **4. Data Transfer Configuration**
Edit `transfer_run.sh` script:
```bash
vim transfer_run.sh
```

Modify these parameters:
```bash
# File path configuration (channels 0-7)
transfer_file_name[0]="picture19201080yuyv.yuv"
transfer_file_name[1]="picture19201080yuyv.yuv"
transfer_file_name[2]="picture19201080yuyv.yuv"
transfer_file_name[3]="picture19201080yuyv.yuv"
transfer_file_name[4]="pic38402160yuyv.yuv"
transfer_file_name[5]="pic38402160yuyv.yuv"
transfer_file_name[6]="pic38402160yuyv.yuv"
transfer_file_name[7]="pic38402160yuyv.yuv"

# Transfer type (YUV/RAW)
transfer_type[0]="YUV"
... # Same configuration for other channels
transfer_type[7]="YUV"

# Frame rate control (register addresses)
# 30fps=6666666, 20fps=10000000, etc.
../tools/reg_rw /dev/xdma0_user 0x30024 w 6666666
../tools/reg_rw /dev/xdma0_user 0x30028 w 6666666
... # Same configuration for other channels
```

---

#### **5. Driver Loading and Data Transfer**
```bash
# Load driver
sudo ./load_driver.sh

# Start data transfer
sudo ./transfer_run.sh

# Stop transfer
sudo ./stop.sh
```

---

### **Key Notes**
1. **YUV File Preparation**:
   - Prepare these resolution files:
     - `picture19201080yuyv.yuv` (1920x1080)
     - `pic38402160yuyv.yuv` (3840x2160)

2. **Frame Rate Registers**:
   | Frame Rate | Register Value |
   |------------|----------------|
   | 30fps      | 6666666        |
   | 20fps      | 10000000       |
   | 15fps      | 13333333       |

3. **Channel Allocation**:
   - Channels 0-3 → 1080p YUV
   - Channels 4-7 → 4K YUV

4. **Hardware Requirements**:
   - Requires XDMA device `/dev/xdma0_user`
   - Ensure sufficient memory for high-resolution video streams

> Frame rate can be dynamically adjusted by modifying register values. Monitor system resource usage during initial execution.
