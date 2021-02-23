LICENSE = "LGPLv2.1"
SRC_URI = "file://bayer.c \
           file://libv4lconvert_bayer.h \
          "
LIC_FILES_CHKSUM = "file://bayer.c;beginline=8;endline=20;md5=982f4923a9641473b07ac3fb0b901898"
ATTRIBUTION = "file://bayer.c;beginline=2;endline=27"

S = "${WORKDIR}"

do_compile() {
  ${CC} ${LDFLAGS} -shared -fPIC -Wl,-soname,libv4lconvert_bayer.so.0 -o libv4lconvert_bayer.so.0.0.0 bayer.c
}

do_install() {
  install -d ${D}${libdir}
  install -m 0755 libv4lconvert_bayer.so* ${D}${libdir}
  ln -s libv4lconvert_bayer.so.0.0.0 ${D}${libdir}/libv4lconvert_bayer.so
  install -d ${D}${includedir}
  install -m 0644 libv4lconvert_bayer.h ${D}${includedir}
}


