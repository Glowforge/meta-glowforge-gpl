FILESEXTRAPATHS_append := "${THISDIR}/${PN}"

RDEPENDS_${PN} = "glowforge-system-utils"

# Remove save-rtc.sh since /etc is readonly and we rely on NTP for
# the correct time anyway.
do_install_append() {
  rm ${D}${sysconfdir}/init.d/save-rtc.sh
  rm ${D}${sysconfdir}/init.d/mountnfs.sh
  rm ${D}${sysconfdir}/init.d/umountnfs.sh
  rm ${D}${sysconfdir}/init.d/rmnologin.sh
  update-rc.d -r ${D} save-rtc.sh remove
  update-rc.d -r ${D} mountnfs.sh remove
  update-rc.d -r ${D} umountnfs.sh remove
  update-rc.d -r ${D} rmnologin.sh remove
}
