FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://defconfig \
           file://imx6dl-glowforge-v110.dts \
           file://imx6dl-glowforge-v12.dts \
           file://imx6dl-glowforge-v13.dts \
           file://imx6dl-glowforge-v14.dts \
           file://imx6dl-glowforge-v16.dts \
           file://imx6dl-glowforge-v18.dts \
           file://imx6dl-glowforge-v19.dts \
           file://imx6dl-glowforge-v20.dts \
           file://0001-bring-back-sys-fsl_otp-driver.patch \
           file://0002-arm-imx-properly-set-mac-addr-from-fuses.patch \
           file://0003-spi-imx-set-ECSPIx_PERIODREG-based-on-delay_usecs.patch \
           file://0004-pwm-imx-additional-prescaler-via-device-tree.patch \
           file://0005-arm-imx-add-EPIT-API-enhancements.patch \
           file://0006-SDMA-API-enhancements-1-of-2.patch \
           file://0007-SDMA-API-enhancements-2-of-2.patch \
           file://0009-dual-ov5648-camera-support.patch \
           file://0010-SDMA-API-invokes-callback-from-hard-irq-context.patch \
           file://0011-mxc-ipu3-don-t-logspam-when-CONFIG_PM-is-off.patch \
           file://0012-mxc_v4l2_capture-can-compile-without-CONFIG_FB.patch \
           file://0013-imx-sdma-define-IMX_SDMA_CALLBACK_IN_HARDIRQ.patch \
           file://0014-watchdog-add-verbose-pretimeout-log.patch \
"


# GROSS HACK to make devtool work. Devtool copies the files in SRC_URI to a separate
# `oe-local-files` directory outside the work directory, to avoid polluting the
# source tree. Devtool accounts for this by adding adds this directory with a
# FILESPATH_prepend line in the bbappend file it creates... but for recipes inheriting
# from kernel-yocto, devtool runs do_configure BEFORE it's updated the bbappend file!
# So bitbake fails at the configure step because it can't find the defconfig file in
# any of the search paths.
# See: https://lists.yoctoproject.org/g/meta-freescale/topic/75903226#24352
# There doesn't seem to be a solution online.
# This may have been fixed post-dunfell.
do_preconfigure_prepend () {
	cp ${WORKDIR}/../oe-local-files/defconfig ${WORKDIR} || true
}

# GROSS HACK PART 2: because running `devtool modify` generates .config, the
# kernel makefile complains about the source tree not being clean and fails, which
# makes bitbake fail when trying to build the kernel.
do_configure_prepend() {
	rm -f ${S}/.config
}

do_compile_prepend () {
	cp ${WORKDIR}/*.dts ${S}/arch/${ARCH}/boot/dts
}

SRCBRANCH = "imx_5.4.3_2.0.0"
LOCALVERSION = "-2.0.0"
SRCREV = "6ea635c2f9b08dc75ffef7d6262a2c6df7afa4e1"

LINUX_VERSION = "5.4.3"

# don't append git hash to kernel version number
SCMVERSION = ""
LOCALVERSION = ""

COMPATIBLE_MACHINE = "(glowforge)"
