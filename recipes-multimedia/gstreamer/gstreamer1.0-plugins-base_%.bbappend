PACKAGECONFIG = ""

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-update-gstappsink.patch"
