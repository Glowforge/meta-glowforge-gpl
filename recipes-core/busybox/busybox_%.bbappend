SRC_URI += "\
    file://enable-runit.cfg \
    file://disable-telnet-tftp-wget.cfg \
    file://disable-misc.cfg \
    file://dhcp-leases-in-data-partition.cfg \
    file://add-shred.cfg \
    file://niceties.cfg \
    file://udhcpd.conf \
    file://add-shred-utility.patch \
"

do_install_append() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/udhcpd.conf ${D}${sysconfdir}
}

FILES_${PN}-udhcpd += "\
    ${sysconfdir}/udhcpd.conf \
"

# Don't start udhcpd at boot.
# Removes /etc/rcX.d symlinks, init script is not deleted.
INITSCRIPT_PACKAGES_remove = "${PN}-udhcpd"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
