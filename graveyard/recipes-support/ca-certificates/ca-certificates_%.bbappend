FILESEXTRAPATHS_append := "${THISDIR}/${PN}"

SRC_URI_append = " \
    file://0003-update-ca-certificates-use-c_rehash.patch \
    file://0010-update-ca-certificates-relative-pem-symlinks.patch \
    "

pkg_postinst_${PN}_prepend() {
    # Add certificates provided by other recipes before updating
    cd $D/etc
    cat ca-certificates.conf* > ca-certificates-conf-combined
    mv ca-certificates-conf-combined ca-certificates.conf
    rm -f ca-certificates.conf?*
}

