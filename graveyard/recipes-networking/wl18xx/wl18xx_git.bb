# from module.bbclass
inherit module-base
addtask make_scripts after do_patch before do_compile
EXTRA_OEMAKE += "KERNEL_SRC=${STAGING_KERNEL_DIR}"
DEPENDS += "virtual/kernel openssl libnl"
do_make_scripts[lockfiles] = "${TMPDIR}/kernel-scripts.lock"
do_make_scripts[deptask] = "do_populate_sysroot"
do_unpack[deptask] = "do_populate_sysroot"

DESCRIPTION = "Drivers for TI WL18xx series wifi modules"
HOMEPAGE = "http://processors.wiki.ti.com/index.php/WL18xx"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE_wl18xx;md5=d7810fab7487fb0aad327b76f1be7cd7 \
                    file://LICENSE_wl18xx_fw;md5=4977a0fe767ee17765ae63c435a32a9e"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"

SRCBRANCH = "master"
SRCREV = "a45a32efb204941a18f06e1fd20c3374dfd7f359"

# the first two patches need to be applied prior to do_patch
SRC_URI = "git://git.ti.com/wilink8-wlan/build-utilites.git \
           file://0001-fix-init-script.patch;apply=no \
           file://0002-disable-unneeded-repos.patch;apply=no \
           file://0004-fix-ti-utils-makefiles.patch \
           file://wl18xx-conf.bin \
           file://LICENSE_wl18xx;subdir=git \
           file://LICENSE_wl18xx_fw;subdir=git"

S = "${WORKDIR}/git"

do_unpack_append() {
    bb.build.exec_func('do_pre_patch', d)
    bb.build.exec_func('do_run_init_script', d)
}

do_pre_patch() {
    patch -p1 < ../0001-fix-init-script.patch
    patch -p1 < ../0002-disable-unneeded-repos.patch
}

do_run_init_script() {
    # create setup-env file
    mkdir -p fs
    dollar="\$"
    echo "\
export ROOTFS=${S}/fs
export TOOLCHAIN_PATH=${STAGING_BINDIR_TOOLCHAIN}
export KERNEL_PATH=${STAGING_KERNEL_BUILDDIR}
export CFLAGS=\"${dollar}{CFLAGS} ${TUNE_CCARGS} --sysroot=${STAGING_DIR_TARGET}\"
export CXXFLAGS=\"${dollar}{CXXFLAGS} ${TUNE_CCARGS} --sysroot=${STAGING_DIR_TARGET}\"
export CPPFLAGS=\"${dollar}{CPPFLAGS} ${BUILDSDK_CPPFLAGS} -I${STAGING_INCDIR}/libnl3\"
export LDFLAGS=\"${dollar}{LDFLAGS} ${BUILDSDK_LDFLAGS} --sysroot=${STAGING_DIR_TARGET}\"
export CROSS_COMPILE=${TARGET_PREFIX}
export ARCH=${TARGET_ARCH}
" > setup-env

    # initialize the workspace
    bash ./build_wl18xx.sh init R8.7_SP3
    # use more recent wifi firmware (FW-818)
    # (TODO 9/14/18 matt: remove this once next version of driver
    # is officially released.)
    cd src/fw_download
    # pin at FW version 8.9.0.0.79
    git checkout d153edae2a75393937da43159b7e6251c2cd01b6
}

do_compile() {
    # need to override CC and LD so the 'conf' utility to build with the right options for kernel modules
    # need to not treat date-time as an error in order to build successfully
    CC="${KERNEL_CC}" LD="${KERNEL_LD}" KCFLAGS="-Wno-error=date-time" bash ./build_wl18xx.sh modules
    bash ./build_wl18xx.sh utils
}

do_install() {
    bash ./build_wl18xx.sh firmware
    bash ./build_wl18xx.sh scripts
    cp -v -p -r ${S}/fs/* ${D}
    cp -v -p -r ../wl18xx-conf.bin ${D}/${base_libdir}/firmware/ti-connectivity
}

FILES_${PN} += "\
    ${base_libdir} \
    ${libdir} \
"

EXPORT_FUNCTIONS do_run_init_script
