# Add the config file, and only include the driver we need
FILESEXTRAPATHS_append := "${THISDIR}/files"
SRC_URI += "file://wl18xx-conf.bin"

FILES_${PN}-wl18xx = " \
  ${nonarch_base_libdir}/firmware/ti-connectivity/wl18xx-fw-4.bin \
  ${nonarch_base_libdir}/firmware/ti-connectivity/wl18xx-conf.bin \
"

# We don't need anything from wlcommon.
RDEPENDS_${PN}-wl18xx = "${PN}-ti-connectivity-license"

do_install_append() {
    install -d ${D}/${base_libdir}/firmware/ti-connectivity
    install -m 0644 ../wl18xx-conf.bin ${D}/${base_libdir}/firmware/ti-connectivity
}

