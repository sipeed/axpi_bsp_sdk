#!/bin/sh

set -e

echo "make axp ..."

PROJECT=$1
VERSION=$2
CHIP_NAME=${PROJECT%\_*}
CHIP_GROUP=${CHIP_NAME:0:5}X
VERSION_EXT=${VERSION}_$(date "+%Y%m%d%H%M%S")

echo $1 $2

if [ "$PROJECT" = "AX620_slave" ]; then
AXP_NAME=${PROJECT}_${VERSION_EXT}.pac
else
AXP_NAME=${PROJECT}_${VERSION_EXT}.axp
fi

LOCAL_PATH=$(pwd)
HOME_PATH=$LOCAL_PATH/..
OUTPUT_PATH=$LOCAL_PATH/out
IMG_PATH=$OUTPUT_PATH/$PROJECT
AXP_PATH=$OUTPUT_PATH/$AXP_NAME

TOOL_PATH=$HOME_PATH/tools/mkaxp/make_axp.py
if [ "$PROJECT" = "AX620_slave" ]; then
TOOL_PATH=$HOME_PATH/tools/mkaxp/make_pac.py
XML_PATH=$HOME_PATH/tools/mkaxp/${CHIP_GROUP}_SLAVE.xml
elif [ "$PROJECT" = "AX620_nand" ] || [ "$PROJECT" = "AX620_nor" ] || [ "$PROJECT" = "AX620_38board" ]; then
XML_PATH=$HOME_PATH/tools/mkaxp/${CHIP_GROUP}_NAND.xml
elif [ "$PROJECT" = "AX620U_38board" ] || [ "$PROJECT" = "AX620U_nand" ]; then
XML_PATH=$HOME_PATH/tools/mkaxp/${CHIP_GROUP}_NAND.xml
else
XML_PATH=$HOME_PATH/tools/mkaxp/$CHIP_GROUP.xml
fi

# rm -f $OUTPUT_PATH/${PROJECT}_${VERSION}*.axp

if [ "$CHIP_GROUP" = "AX620X" ]; then
if [ "$PROJECT" = "AX620_nand" ] || [ "$PROJECT" = "AX620_38board" ] || [ "$PROJECT" = "AX620U_38board" ] || [ "$PROJECT" = "AX620U_nand" ] ; then
	DTB_PATH=$IMG_PATH/${PROJECT}.dtb
	EIP_PATH=$IMG_PATH/eip.bin
	FDL1_PATH=$IMG_PATH/fdl_${PROJECT}_signed.bin
	FDL2_PATH=$IMG_PATH/fdl2_signed.bin
	UBOOT_PATH=$IMG_PATH/u-boot_signed.bin
	KERNEL_PATH=$IMG_PATH/uImage
	ROOTFS_PATH=$IMG_PATH/rootfs_soc_opt.ubi
	SPL_PATH=$IMG_PATH/spl_${PROJECT}_signed.bin
	python3 $TOOL_PATH -p $CHIP_GROUP -v $VERSION_EXT -x $XML_PATH -o $AXP_PATH $EIP_PATH $FDL1_PATH $FDL2_PATH $UBOOT_PATH $DTB_PATH $KERNEL_PATH $ROOTFS_PATH $SPL_PATH
	echo "make AX620_nand axp done"
elif [ "$PROJECT" = "AX620_nor" ]; then
	DTB_PATH=$IMG_PATH/${PROJECT}.dtb
	EIP_PATH=$IMG_PATH/eip.bin
	FDL1_PATH=$IMG_PATH/fdl_${PROJECT}_signed.bin
	FDL2_PATH=$IMG_PATH/fdl2_signed.bin
	UBOOT_PATH=$IMG_PATH/u-boot_signed.bin
	KERNEL_PATH=$IMG_PATH/uImage
	ROOTFS_PATH=$IMG_PATH/rootfs.jffs2
	SPL_PATH=$IMG_PATH/spl_${PROJECT}_signed.bin
	python3 $TOOL_PATH -p $CHIP_GROUP -v $VERSION_EXT -x $XML_PATH -o $AXP_PATH $EIP_PATH $FDL1_PATH $FDL2_PATH $UBOOT_PATH $DTB_PATH $KERNEL_PATH $ROOTFS_PATH $SPL_PATH
	echo "make AX620_nor axp done"
elif [ "$PROJECT" = "AX620_slave" ]; then
	FDL_PATH=$IMG_PATH/fdl_${PROJECT}.bin
	KERNEL_PATH=$IMG_PATH/Image
	DTB_PATH=$IMG_PATH/${PROJECT}.dtb
	ROOTFS_PATH=$IMG_PATH/rootfs.ext4
	rm -rf $OUTPUT_PATH/${PROJECT}*.pac
	python3 $TOOL_PATH -p $CHIP_GROUP -v $VERSION -x $XML_PATH -o $AXP_PATH $FDL_PATH $KERNEL_PATH $DTB_PATH $ROOTFS_PATH
	echo "make pac done"
else
	DTB_PATH=$IMG_PATH/${PROJECT}.dtb
	EIP_PATH=$IMG_PATH/eip.bin
	FDL1_PATH=$IMG_PATH/fdl_${PROJECT}_signed.bin
	FDL2_PATH=$IMG_PATH/fdl2_signed.bin
	UBOOT_PATH=$IMG_PATH/u-boot_signed.bin
	KERNEL_PATH=$IMG_PATH/uImage
	ROOTFS_PATH=$IMG_PATH/rootfs_sparse.ext4
	PARAM_PATH=$IMG_PATH/param_sparse.ext4
	SOC_PATH=$IMG_PATH/soc_sparse.ext4
	OPT_PATH=$IMG_PATH/opt_sparse.ext4
	SPL_PATH=$IMG_PATH/spl_${PROJECT}_signed.bin
	BOOT_PATH=$IMG_PATH/boot.img
	if [ ! -f $BOOT_PATH ];then
		echo "make boot.img file..."
		python3 $HOME_PATH/tools/imgsign/boot_AX620_sign.py -i $KERNEL_PATH -o $BOOT_PATH
	fi
	python3 $TOOL_PATH -p $CHIP_GROUP -v $VERSION_EXT -x $XML_PATH -o $AXP_PATH $EIP_PATH $FDL1_PATH $FDL2_PATH $UBOOT_PATH $DTB_PATH $BOOT_PATH $PARAM_PATH $SOC_PATH $OPT_PATH $ROOTFS_PATH $SPL_PATH
	echo "make axp done"
fi
else
	echo "make axp failed"
fi






