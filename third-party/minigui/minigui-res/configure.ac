dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)
AC_INIT(minigui-res, 5.0.0)
AC_CONFIG_SRCDIR(bmp/classic_check_button.bmp)
AM_INIT_AUTOMAKE

dnl ========================================================================
dnl Checks for programs.
AC_PROG_MAKE_SET

AC_CHECK_FILE(font/extra-fonts/9x18-iso8859-1.bin, guess_mgedition="svn", guess_mgedition="rel")

if test "x$guess_mgedition" = "xrel"; then
  AC_CHECK_FILE(font/song-12-gb2312.bin, with_mgedition="se", with_mgedition="ve")
else
  AC_ARG_WITH(mgedition, [  --with-mgedition=ve/se/pe/ee])
fi

ADOBE_FONTS=" \
AdobeVFPrototype.ttf \
SourceCodeVariable-Italic.ttf \
SourceCodeVariable-Roman.ttf \
SourceHanSans-Regular.ttc \
SourceHanSerif-Regular.ttc \
SourceSansPro-Black.ttf \
SourceSansPro-BlackIt.ttf \
SourceSansPro-Bold.ttf \
SourceSansPro-BoldIt.ttf \
SourceSansPro-ExtraLight.ttf \
SourceSansPro-ExtraLightIt.ttf \
SourceSansPro-It.ttf \
SourceSansPro-Light.ttf \
SourceSansPro-LightIt.ttf \
SourceSansPro-Regular.ttf \
SourceSansPro-Semibold.ttf \
SourceSansPro-SemiboldIt.ttf \
SourceSerifPro-Black.otf \
SourceSerifPro-Black.ttf \
SourceSerifPro-BlackIt.otf \
SourceSerifPro-BlackIt.ttf \
SourceSerifPro-Bold.otf \
SourceSerifPro-Bold.ttf \
SourceSerifPro-BoldIt.otf \
SourceSerifPro-BoldIt.ttf \
SourceSerifPro-ExtraLight.otf \
SourceSerifPro-ExtraLight.ttf \
SourceSerifPro-ExtraLightIt.otf \
SourceSerifPro-ExtraLightIt.ttf \
SourceSerifPro-It.otf \
SourceSerifPro-It.ttf \
SourceSerifPro-Light.otf \
SourceSerifPro-Light.ttf \
SourceSerifPro-LightIt.otf \
SourceSerifPro-LightIt.ttf \
SourceSerifPro-Regular.otf \
SourceSerifPro-Regular.ttf \
SourceSerifPro-Semibold.otf \
SourceSerifPro-Semibold.ttf \
SourceSerifPro-SemiboldIt.otf \
SourceSerifPro-SemiboldIt.ttf \
"

case "$with_mgedition" in
  ve)
    DEV_FONTS="unifont_160_50.upf"
    ;;

  se)
    DEV_FONTS="\
6x12-iso8859-1.bin \
8x16-iso8859-1.bin \
12x24-iso8859-1.bin \
song-12-gb2312.bin \
song-16-gb2312.bin \
song-24-gb2312.bin \
fmfsong-12-gb2312.bin \
fmfsong-14-gb2312.bin \
fmfsong-16-gb2312.bin \
fmfsong-24-gb2312.bin \
fmhei-16-gb2312.bin \
fmhei-18-gb2312.bin \
fmhei-20-gb2312.bin \
fmhei-24-gb2312.bin \
Courier-rr-10-15.vbf \
Courier-rr-8-13.vbf \
Helvetica-rr-11-12.vbf \
Helvetica-rr-15-16.vbf \
Times-rr-10-12.vbf \
Times-rr-13-15.vbf \
fmfsong-gb2312-12.upf \
fmfsong-gb2312-13.upf \
fmfsong-gb2312-14.upf \
fmfsong-gb2312-15.upf \
fmfsong-gb2312-16.upf \
fmfsong-gb2312-18.upf \
fmfsong-gb2312-20.upf \
fmhei-gbk-16.upf \
fmhei-gbk-18.upf \
fmhei-gbk-20.upf \
fmkai-gb2312-12.upf \
fmkai-gb2312-13.upf \
fmkai-gb2312-14.upf \
fmkai-gb2312-15.upf \
fmkai-gb2312-16.upf \
fmkai-gb2312-18.upf \
fmkai-gb2312-20.upf \
fmsong-gbk-12.upf \
fmsong-gbk-13.upf \
fmsong-gbk-14.upf \
fmsong-gbk-15.upf \
fmsong-gbk-16.upf \
fmsong-gbk-18.upf \
courier-mr-8.upf \
courier-mr-10.upf \
courier-mr-12.upf \
courier-mr-14.upf \
courier-mr-18.upf \
courier-mr-24.upf \
helvetica-mr-8.upf \
helvetica-mr-10.upf \
helvetica-mr-12.upf \
helvetica-mr-14.upf \
helvetica-mr-18.upf \
helvetica-mr-24.upf \
lucida-mr-8.upf \
lucida-mr-10.upf \
lucida-mr-12.upf \
lucida-mr-14.upf \
lucida-mr-18.upf \
lucida-mr-19.upf \
lucida-mr-24.upf \
times-mr-8.upf \
times-mr-10.upf \
times-mr-12.upf \
times-mr-14.upf \
times-mr-18.upf \
times-mr-24.upf \
micro_40_50.upf \
smoothtimes_100_50.upf \
unifont_160_50.upf"
    ;;

  *)
    DEV_FONTS=" \
6x12-iso8859-1.bin \
8x16-iso8859-1.bin \
12x24-iso8859-1.bin \
batang-12-ksc5601.bin \
fixed-16-jiskan.bin \
mincho-12-jiskan.bin \
mincho-16-ksc5601.bin \
gothic-12-jiskan.bin \
gothic-6x12-jisx0201.bin \
gothic-6x12rk.bin \
fmfsong-12-gb2312.bin \
fmfsong-14-gb2312.bin \
fmfsong-16-gb2312.bin \
fmfsong-24-gb2312.bin \
ming-12-big5.bin \
ming-16-big5.bin \
ming-24-big5.bin \
song-12-gb2312.bin \
song-16-gb2312.bin \
song-24-gb2312.bin \
fmhei-16-gb2312.bin \
fmhei-18-gb2312.bin \
fmhei-20-gb2312.bin \
fmhei-24-gb2312.bin \
Helvetica-rr-11-12.vbf \
Helvetica-rr-15-16.vbf \
Times-rr-10-12.vbf \
Times-rr-13-15.vbf \
Courier-rr-10-15.vbf \
Courier-rr-8-13.vbf \
naskhi-18-21-iso8859-6.vbf \
fixed-10-20-iso8859-8.vbf \
fmfsong-gb2312-12.upf \
fmfsong-gb2312-13.upf \
fmfsong-gb2312-14.upf \
fmfsong-gb2312-15.upf \
fmfsong-gb2312-16.upf \
fmfsong-gb2312-18.upf \
fmfsong-gb2312-20.upf \
fmhei-gbk-16.upf \
fmhei-gbk-18.upf \
fmhei-gbk-20.upf \
fmkai-gb2312-12.upf \
fmkai-gb2312-13.upf \
fmkai-gb2312-14.upf \
fmkai-gb2312-15.upf \
fmkai-gb2312-16.upf \
fmkai-gb2312-18.upf \
fmkai-gb2312-20.upf \
fmsong-gbk-12.upf \
fmsong-gbk-13.upf \
fmsong-gbk-14.upf \
fmsong-gbk-15.upf \
fmsong-gbk-16.upf \
fmsong-gbk-18.upf \
courier-mr-8.upf \
courier-mr-10.upf \
courier-mr-12.upf \
courier-mr-14.upf \
courier-mr-18.upf \
courier-mr-24.upf \
helvetica-mr-8.upf \
helvetica-mr-10.upf \
helvetica-mr-12.upf \
helvetica-mr-14.upf \
helvetica-mr-18.upf \
helvetica-mr-24.upf \
lucida-mr-8.upf \
lucida-mr-10.upf \
lucida-mr-12.upf \
lucida-mr-14.upf \
lucida-mr-18.upf \
lucida-mr-19.upf \
lucida-mr-24.upf \
times-mr-8.upf \
times-mr-10.upf \
times-mr-12.upf \
times-mr-14.upf \
times-mr-18.upf \
times-mr-24.upf \
micro_40_50.upf \
smoothtimes_100_50.upf \
unifont_160_50.upf"
    ;;
esac

AC_SUBST(ADOBE_FONTS)
AC_SUBST(DEV_FONTS)

AC_OUTPUT(
Makefile
bmp/Makefile
cursor/Makefile
icon/Makefile
font/Makefile
adobe-fonts/Makefile
)

