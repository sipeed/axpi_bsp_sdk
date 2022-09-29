
Usage:

​sample_venc  [options]   -i   inputfile

## Options for sample

```
   -H	--help							help information
  -i[s] --input                    		Read input video sequence from file. [input.yuv]
  -o[s] --output                   		Write output HEVC/H.264 stream to file. [NULL]

  -f[n] --outputFrameRate         		1..1048575 Output picture rate numerator. [30]

  -j[n] --inputFrameRate            	1..1048575 Input picture rate numerator. [30]

  -E[n] --frameNum                  	the frame number want to encode. [-1]
  -Z[n] --syncType                 		send/get frame/stream mode; -1: block mode 0: none block >0: time wait. [-1]
  -N[n] --chnNum                    total encode channel number,(default 2)
```



## Parameters affecting input frame and encoded frame resolutions and cropping:

```
  -w[n] --width                     	Width of input image in pixels.
  -h[n] --height                   		Height of input image in pixels.

  -X[n] --horOffsetSrc             		image horizontal cropping offset, must be even. [0]
  -Y[n] --verOffsetSrc                 	image vertical cropping offset, must be even. [0]
  -x[n] --cropWidth                     Height of encoded image
  -y[n] --cropHeight                    Width of encoded image

  --enableCrop                			enable crop encode, 0: disable 1: enable crop
```



## Parameters  for pre-processing frames before encoding:

```
  -l[n] --inputFormat              Input YUV format. [0]
                                     0 - YUV420 planar CbCr (IYUV/I420)
                                     1 - YUV420 semiplanar CbCr (NV12)
                                     2 - YUV420 semiplanar CrCb (NV21)
                                     3 - YUYV422 interleaved (YUYV/YUY2)
                                     4 - UYVY422 interleaved (UYVY/Y422)
                                     5 - RGB565 16bpp
                                     6 - BGR565 16bpp
                                     7 - RGB555 16bpp
                                     8 - BGR555 16bpp
                                     9 - RGB444 16bpp
                                     10 - BGR444 16bpp
                                     11 - RGB888 32bpp
                                     12 - BGR888 32bpp
                                     13 - RGB101010 32bpp
                                     14 - BGR101010 32bpp
                                     38 - FBC    YUV420 semiplannar 8 bit tile 64x2 \n"
```


```
  -r[n] --rotation                 Rotate input image. [0]
                                     0 - disabled
                                     1 - 90 degrees right
                                     2 - 90 degrees left
                                     3 - 180 degrees right
```




## Parameters  affecting the output stream and encoding tools:

```
  -P[n] --profile                  HEVC encoder profile [0]
                                     HEVC encoder only supports Main profile, Main Still Picture profile, and Main 10 profile.
                                     0 - Main profile
                                     1 - Main Still Picture profile
                                     2 - Main 10 profile
                                     3 - MAINREXT profile

​                                   H.264 encoder profile [11]
​                                   H.264 only supports Baseline, Main, High, High 10 profiles.
​                                     9  - Baseline profile
​                                     10 - Main profile
​                                     11 - High profile
​                                     12 - High 10 profile
```



```
  -L[n] --level                    HEVC level. 180 = level 6.0*30 [180]
                                     HEVC HW only supports less than or equal to level 5.1 High tier in real-time.
                                     For levels 5.0 and 5.1, support resolutions up to 4096x2048 with 4K@60fps performance.
                                     For levels greater than level 5.1, support in non-realtime mode.
                                     Each level has resolution and bitrate limitations:
                                     30  - level 1.0  QCIF      (176x144)       128 kbps
                                     60  - level 2.0  CIF       (352x288)       1.5 Mbps
                                     63  - level 2.1  Q720p     (640x360)       3.0 Mbps
                                     90  - level 3.0  QHD       (960x540)       6.0 Mbps
                                     93  - level 3.1  720p HD   (1280x720)      10.0 Mbps
                                     120 - level 4.0  2Kx1080   (2048x1080)     12.0 Mbps   High tier 30 Mbps
                                     123 - level 4.1  2Kx1080   (2048x1080)     20.0 Mbps   High tier 50 Mbps
                                     150 - level 5.0  4096x2160 (4096x2160)     25.0 Mbps   High tier 100 Mbps
                                     153 - level 5.1  4096x2160 (4096x2160)     40.0 Mbps   High tier 160 Mbps
                                     156 - level 5.2  4096x2160 (4096x2160)     60.0 Mbps   High tier 240 Mbps
                                     180 - level 6.0  8192x4320 (8192x4320)     60.0 Mbps   High tier 240 Mbps
                                     183 - level 6.1  8192x4320 (8192x4320)     120.0 Mbps  High tier 480 Mbps
                                     186 - level 6.2  8192x4320 (8192x4320)     240.0 Mbps  High tier 800 Mbps
```



                                    H.264 level. 51 = Level 5.1 [51]
                                     10 - H264_LEVEL_1
                                     99 - H264_LEVEL_1_b
                                     11 - H264_LEVEL_1_1
                                     12 - H264_LEVEL_1_2
                                     13 - H264_LEVEL_1_3
                                     20 - H264_LEVEL_2
                                     21 - H264_LEVEL_2_1
                                     22 - H264_LEVEL_2_2
                                     30 - H264_LEVEL_3
                                     31 - H264_LEVEL_3_1
                                     32 - H264_LEVEL_3_2
                                     40 - H264_LEVEL_4
                                     41 - H264_LEVEL_4_1
                                     42 - H264_LEVEL_4_2
                                     50 - H264_LEVEL_5
                                     51 - H264_LEVEL_5_1
                                     52 - H264_LEVEL_5_2
                                     60 - H264_LEVEL_6
                                     61 - H264_LEVEL_6_1
                                     62 - H264_LEVEL_6_2

        --tier                       HEVC encoder tier [0]
                                     HEVC encoder only supports Main tier and High tier
                                     0 - Main tier
                                     1 - High tier

```
  -k[n] --videoRange               0..1 Video signal sample range in encoded stream. [0]
                                     					0 - Y range in [16..235]; Cb, Cr in [16..240]
                                     					1 - Y, Cb, Cr range in [0..255]

  -z[s] --userData                 SEI User data file name. File is read and inserted
                                     as an SEI message before the first frame.
```



        --codecFormat              Select Video Codec Format  [0]
                                   0: unknown codec format
                                   1: HEVC video format encoding
                                   2: H.264 video format encoding
                                   or by codec name:
                                   hevc: HEVC video format encoding
                                   h264: H.264 video format encoding

```
  -K[n] --enableCabac              0=OFF (CAVLC), 1=ON (CABAC). [1]

  -e[n] --sliceSize                [0..height/ctu_size] slice size in number of CTU rows [0]
                                     					0 to encode each picture in one slice
                                     					[1..height/ctu_size] to encode each slice with N CTU rows

  -D[n] --disableDeblocking        0..1 Disable deblocking filter [0]
                                     					0 = Inloop deblocking filter enabled (better quality)
                                     					1 = Inloop deblocking filter disabled

  -M[n] --enableSao                0..1 Disable or enable SAO filter [1]
                                     						0 = SAO disabled
                                     						1 = SAO enabled
```



        --enableDeblockOverride    0..1 Deblocking override enable flag [0]
                                     0 = Disable override
                                     1 = Enable override

        --deblockOverride          0..1 Deblocking override flag [0]
                                     0 = Do not override filter parameters
                                     1 = Override filter parameters



## Parameters  affecting GOP pattern, rate control and output stream bitrate:

```
  -R[n] --gopLength             Intra-picture rate in frames. [0]
                                   Forces every Nth frame to be encoded as intra frame.
                                     0 = Do not force

  -B[n] --bitRate             target bitrate for rate control, in kbps. [2000]

  -U[n] --picRc                    0=OFF, 1=ON Picture rate control. [0]
                                     Calculates a new target QP for every frame.

  -u[n] --ctbRc                    	 0..3 CTB QP adjustment mode for Rate Control and Subjective Quality. [0]
                                     0 = No CTB QP adjustment.
                                     1 = CTB QP adjustment for Subjective Quality only.
                                     2 = CTB QP adjustment for Rate Control only. (For HwCtbRcVersion >= 1 only)
                                     3 = CTB QP adjustment for both Subjective Quality and Rate Control. (For HwCtbRcVersion >= 1 only)
                                     4 = CTB QP adjustment for Subjective Quality only, reversed.
                                     6 = CTB QP adjustment for both Subjective Quality reversed and Rate Control. (For HwCtbRcVersion >= 1 only)
```



        --blockRCSize              Block size in CTB QP adjustment for Subjective Quality [0]
                                     0=64x64 ,1=32x32, 2=16x16

        --rcQpDeltaRange           Max absolute value of CU/MB QP delta relative to frame QP in CTB RC [10]
                                   0..15 for HwCtbRcVersion <= 1;
                                   0..51 for HwCtbRcVersion > 1.

        --picQpDeltaRange          Min:Max. Qp_Delta range in picture RC.
                                     Min: -10..-1 Minimum Qp_Delta in picture RC. [-2]
                                     Max:  1..10  Maximum Qp_Delta in picture RC. [3]
                                     This range only applies to two neighboring frames of the same
                                     coding type. This range doesn't apply when HRD overflow occurs.

```
  -C[n] --hrdConformance           0..1 HRD conformance checking turn on or off. [0] 0=OFF, 1=ON
                                     Uses standard defined model to limit bitrate variance.

  -c[n] --cpbSize                  HRD Coded Picture Buffer size in bits. [0]
                                     If hrdConformance=1, default value 0 means the max CPB for
                                     current level. Or default value 0 means RC doesn't consider
                                     cpb buffer limit.
```



      --cpbMaxRate               Max local bitrate (bit/s). [0]
                                     If cpbSize is set but cpbMaxRate is not set or < bitrate,
                                        cpbMaxRate is defaultly set to bitrate.
                                     If cpbMaxRate == bitrate, it's CBR mode, else if cpbMaxRate > bitrate it's VBR mode.

```
  -g[n] --bitrateWindow            1..300, Bitrate window length in frames [intraPicRate]
  									Rate control allocates bits for one window and tries to
                                     match the target bitrate at the end of each window.
                                     Typically window begins with an intra frame, but this
                                   is not mandatory.
```



      --gopType                  0..2 GOP Structure. [0]
                                       0  : NormalP
                                       1  : OneLTR
                                       2  : Svc-T

```
  -s[n] --picSkip                  0..1 Picture skip rate control. [0]
                                   0:OFF, 1:ON. Allows rate control to skip frames if needed.

  -q[n] --qpHdr                    -1..51, Initial target QP. [26]
                                   -1 : Encoder calculates initial QP. NOTE: specify as "-q-1"
                                   The initial QP used in the beginning of stream.

  -n[n] --qpMin                    	0..51, Minimum frame header QP for any slices. [0]
  -m[n] --qpMax                    	0..51, Maximum frame header QP for any slices. [51]
        --qpMinI                   	0..51, Minimum frame header QP, overriding qpMin for I slices. [0]
        --qpMaxI                   	0..51, Maximum frame header QP, overriding qpMax for I slices. [51]

  -A[n] --intraQpDelta             	-51..51, Intra QP delta. [-5]
                                    QP difference between target QP and intra frame QP.
```



      --rcMode                   0: CBR 1: VBR 2: AVBR 3: QPMAP 4:FIXQP. [0]

      --gdrDuration              how many frames it will take to do GDR [0]
                                     0 : disable GDR (Gradual decoder refresh),
                                     >0: enable GDR
                                     The starting point of GDR is the frame with type set to VCENC_INTRA_FRAME.
                                     intraArea and roi1Area are used to implement the GDR function. The GDR
                                     begin to work from the second IDR frame.


## Parameters affecting coding:


        --picStride                Y:Cb:Cr stride value

        --roi1Area                 left:top:right:bottom CTB coordinates
        --roi2Area                 left:top:right:bottom CTB coordinates
        --roi3Area                 left:top:right:bottom CTB coordinates
        --roi4Area                 left:top:right:bottom CTB coordinates
        --roi5Area                 left:top:right:bottom CTB coordinates
        --roi6Area                 left:top:right:bottom CTB coordinates
        --roi7Area                 left:top:right:bottom CTB coordinates
        --roi8Area                 left:top:right:bottom CTB coordinates
                                     Specifies the rectangular area of CTBs as Region Of Interest
                                     with lower QP. the CTBs in the edges are all inclusive.

        --roi1DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 1 CTBs. [0]
        --roi2DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 2 CTBs. [0]
        --roi3DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 3 CTBs. [0]
        --roi4DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 4 CTBs. [0]
        --roi5DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 5 CTBs. [0]
        --roi6DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 6 CTBs. [0]
        --roi7DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 7 CTBs. [0]
        --roi8DeltaQp              -30..0 (-51..51 if absolute ROI QP supported), QP delta value for ROI 8 CTBs. [0]

        --roi1Qp                   0..51, absolute QP value for ROI 1 CTBs. [-1]. Negative value means invalid.,
                                     and another value is used to calculate the QP in the ROI area.

        --roi2Qp                   0..51, absolute QP value for ROI 2 CTBs. [-1]
        --roi3Qp                   0..51, absolute QP value for ROI 3 CTBs. [-1]
        --roi4Qp                   0..51, absolute QP value for ROI 4 CTBs. [-1]
        --roi5Qp                   0..51, absolute QP value for ROI 5 CTBs. [-1]
        --roi6Qp                   0..51, absolute QP value for ROI 6 CTBs. [-1]
        --roi7Qp                   0..51, absolute QP value for ROI 7 CTBs. [-1]
        --roi8Qp                   0..51, absolute QP value for ROI 8 CTBs. [-1]
                                   roi1Qp..roi8Qp are only valid when absolute ROI QP is supported. Use either roiDeltaQp or roiQp.

## Parameters setting constant chroma:

        --enableConstChroma        0..1 Enable/Disable setting chroma to a constant pixel value. [0]
                                     0 = Disable.
                                     1 = Enable.
        --constCb                  0..255 for 8-bit [128]; 0..1023 for 10-bit [512]. The constant pixel value for Cb.
        --constCr                  0..255 for 8-bit [128]; 0..1023 for 10-bit [512]. The constant pixel value for Cr.
