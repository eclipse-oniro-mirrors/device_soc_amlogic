#!/bin/bash
#
# Copyright (c) 2022 Unionman Technology Co., Ltd.
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
#

set -e

BOOT_LINUX=${1}/kernel/src_tmp/linux-5.10
OUT_IMAGE=${1}/unionpi_tiger/packages/phone/images
MKBOOT_IMG=${1}/../device/board/unionman/unionpi_tiger/common/tools/linux/mkbootimg

function make_boot_image()
{
	${MKBOOT_IMG} --kernel ${BOOT_LINUX}/arch/arm64/boot/Image.gz --base 0x0 --kernel_offset 0x1080000 --cmdline "" --ramdisk  ${OUT_IMAGE}/ramdisk.img --second ${OUT_IMAGE}/dtb.img --output ${OUT_IMAGE}/boot.img > /dev/null

	mkdir -p ${BOOT_LINUX}/unionpi_tiger
	cp ${BOOT_LINUX}/arch/arm64/boot/Image.gz ${BOOT_LINUX}/unionpi_tiger

	return $?
}

make_boot_image
