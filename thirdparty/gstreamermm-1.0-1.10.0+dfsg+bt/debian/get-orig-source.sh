#!/bin/sh

set -ex

UPSTREAM_VERSION=$2
ORIG_TARBALL=$3

REAL_TARBALL=`readlink -f ${ORIG_TARBALL}`

WORKING_DIR=`dirname ${ORIG_TARBALL}`

ORIG_TARBALL_DFSG=`echo ${ORIG_TARBALL} | sed -e "s/\(${UPSTREAM_VERSION}\)\(\.orig\)/\1+dfsg\2/g"`
ORIG_TARBALL_DIR=`echo ${ORIG_TARBALL_DFSG} | sed -e "s/_\(${UPSTREAM_VERSION}\)/-\1/g" -e "s/\.tar\.gz//g"`
ORIG_TARBALL_DIR_STRIP=`basename ${ORIG_TARBALL_DIR}`

mkdir -p ${ORIG_TARBALL_DIR}
tar --directory=${ORIG_TARBALL_DIR} --strip 1 -xJf ${REAL_TARBALL} || exit 1 
rm -f ${ORIG_TARBALL} ${REAL_TARBALL}
# delete documentation as the source is missing
rm -f ${ORIG_TARBALL_DIR}/docs/reference/gstreamermm-1.0.devhelp2
rm -f ${ORIG_TARBALL_DIR}/docs/reference/gstreamermm-1.0.tag
rm -rf ${ORIG_TARBALL_DIR}/docs/reference/html
tar --remove-files --directory ${WORKING_DIR} -cJf ${ORIG_TARBALL_DFSG} ${ORIG_TARBALL_DIR_STRIP} || exit 1

exit 0
