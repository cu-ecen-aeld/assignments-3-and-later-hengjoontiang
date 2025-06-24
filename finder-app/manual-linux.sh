#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

#
#Take note to rm the 3 staging folders to avoid issues before rebuild
#
#


set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
#git://git.launchpad.net/~ubuntu-kernel-test/ubuntu/+source/linux/+git/mainline-crack cod/mainline/v5.18.5
KERNEL_VERSION=v5.15.185
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
ROOTFS=${OUTDIR}/rootfs
SYSTEMFS=${OUTDIR}/systemfs
CURRENT_DIR=$(pwd)
echo "CURRENT_DIR="${CURRENT_DIR}


if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi
#sudo rm  -rf ${OUTDIR}/linux-stable
#sudo rm  -rf ${OUTDIR}/busybox
#clear it to avoid any git issues
#sudo rm  -rf ${OUTDIR}

mkdir -p ${OUTDIR}
#https://medium.com/@fprotopapa/embedded-linux-from-scratch-quick-easy-on-qemu-87e761834b51
cd "$OUTDIR"

if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
#https://www.subrat.info/build-kernel-and-userspace/
#SYSROOT
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    #https://medium.com/@kiky.tokamuro/creating-initramfs-5cca9b524b5a
    # TODO: Add your kernel build steps here
    
    make ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  mrproper
    make ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  defconfig
    #make ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  menuconfig
    make -j4 ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  all
    make ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  modules
    make ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  dtbs
    
    
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/Image

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf "${OUTDIR}/rootfs"
fi

# TODO: Create necessary base directories
mkdir "${OUTDIR}/rootfs"
pushd "${OUTDIR}/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
popd

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
    
   
else
    cd busybox
    
fi

# TODO: Make and install busybox
#dont use static -- busybox uses dynamic links
#make LDFLAGS=-static ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE} 
make ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE} 
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH}  CROSS_COMPILE=${CROSS_COMPILE}  install 

    
echo "Library dependencies"
cd "${OUTDIR}/rootfs"

echo "CROSS_COMPILE="" ${CROSS_COMPILE}"
pwd
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"
#copy from the tool chain /home/hengjt/Downloads/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64
# TODO: Add library dependencies to rootfs
#actual full-test has no access to the toolchain

#pushd /home/hengjt/Downloads/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib
pushd "${CURRENT_DIR}/dependency"
cp -a lib/ld-linux-aarch64.so.1 "${OUTDIR}/rootfs/lib"
#pushd /home/hengjt/Downloads/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64
cp -a lib64/libm.so.6 lib64/libresolv.so.2 lib64/libc.so.6 "${OUTDIR}/rootfs/lib64"
#cp -a libm.so.6 libresolv.so.2 libc.so.6 "${OUTDIR}/rootfs/lib"
popd
# TODO: Make device nodes
cd "${OUTDIR}/rootfs"
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1 

pushd "${CURRENT_DIR}"
# TODO: Clean and build the writer utility

${CROSS_COMPILE}gcc writer.c -o writer


mkdir -p "${OUTDIR}/rootfs"/conf 
mkdir -p "${OUTDIR}/rootfs"/home/conf 
#mkdir -p "${OUTDIR}/rootfs"/home/finder-app 
#mkdir -p "${OUTDIR}/rootfs"/home/finder-app/conf 
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -a conf/assignment.txt conf/username.txt "${OUTDIR}/rootfs"/home/conf
cp -a conf/assignment.txt conf/username.txt "${OUTDIR}/rootfs"/conf/
cp -a writer finder-test.sh finder.sh autorun-qemu.sh "${OUTDIR}/rootfs/home"
#cp -a autorun-qemu.sh "${OUTDIR}/rootfs/home"
chmod +x "${OUTDIR}"/rootfs/home/*
popd




# TODO: Chown the root directory
#operation not allowed, check!
#use chroot if no access to root
cd "${OUTDIR}/rootfs"
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
cd "${OUTDIR}/rootfs"
#cd initramfs
find .  | cpio -H newc -ov --owner root:root  > "${OUTDIR}"/initramfs.cpio 
#no need for this line cpio --owner root:root acjieves this
#sudo chown root:root "${OUTDIR}"/initramfs.cpio 
gzip -f "${OUTDIR}"/initramfs.cpio
