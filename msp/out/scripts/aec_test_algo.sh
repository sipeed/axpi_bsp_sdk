#!/bin/sh

cd ref_dir/
sample_audio play 1 > /dev/null &
cd ..
sample_audio cap -r 16000 -p 160 --aec-mode 2 -w 1
