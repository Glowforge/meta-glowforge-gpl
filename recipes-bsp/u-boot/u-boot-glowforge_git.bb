require recipes-bsp/u-boot/u-boot.inc
require u-boot-glowforge.inc

DESCRIPTION = "u-boot for the Glowforge control board"
COMPATIBLE_MACHINE = "(glowforge|glowforge-dev)"

PROVIDES = "u-boot"

SRC_URI += "file://uEnv.txt"

UENV_FILENAME ?= "uEnv-${MACHINE}.txt"

deploy_uenv() {
  install ${WORKDIR}/uEnv.txt ${DEPLOYDIR}/${UENV_FILENAME}
}

do_deploy[postfuncs] += "deploy_uenv"

PACKAGE_ARCH = "${MACHINE_ARCH}"
