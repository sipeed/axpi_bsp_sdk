#!/bin/sh

cd ref_dir/
sample_audio play 1 > /dev/null &
cd ..
/usr/local/bin/tinycap 123.wav -r 16000