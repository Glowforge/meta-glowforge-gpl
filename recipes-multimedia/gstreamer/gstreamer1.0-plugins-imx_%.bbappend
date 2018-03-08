FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://00_imx_gst_add_bggr_support.patch \
            file://01_imx_gst_add_crop_support.patch"

