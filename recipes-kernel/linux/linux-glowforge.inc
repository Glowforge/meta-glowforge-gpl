include recipes-kernel/linux/linux-imx.inc
include recipes-kernel/linux/linux-dtb.inc

DEPENDS += "lzop-native bc-native"

GF_BUILD_BRANCH ?= "glowforge"
SRCREV = "${AUTOREV}"
LOCALVERSION ?= "-${GF_BUILD_BRANCH}"

SRC_URI = "${GF_SRC_URI}/linux-glowforge.git;protocol=ssh;branch=${GF_BUILD_BRANCH} \
           file://mxc_v4l2_capture.conf \
           file://defconfig \
           file://imx6dl-glowforge-v110.dts \
           file://imx6dl-glowforge-v12.dts \
           file://imx6dl-glowforge-v13.dts \
           file://imx6dl-glowforge-v14.dts \
           file://imx6dl-glowforge-v16.dts \
           file://imx6dl-glowforge-v18.dts \
           file://imx6dl-glowforge-v19.dts \
           file://imx6dl-glowforge-v20.dts \
           "

do_compile_prepend () {
	cp ${WORKDIR}/*.dts ${S}/arch/${ARCH}/boot/dts
}

# module configuration
do_install_append() {
  install -d ${D}${sysconfdir}/modprobe.d/
  install -m 0644 ${WORKDIR}/mxc_v4l2_capture.conf ${D}${sysconfdir}/modprobe.d/
}

FILES_${PN} += "${sysconfdir}/modprobe.d/mxc_v4l2_capture.conf"

COMPATIBLE_MACHINE = "(glowforge)"

