#!/bin/sh
cur_path=$(cd "$(dirname $0)";pwd)

process=ipc_demo

pid=$(pidof ${process})
if [ $pid ]; then
  echo "${process} is already running, please check the process(pid: $pid) first."
  exit 1;
fi

if [ -e /tmp/core* ]; then
  echo "exist coredump file under path: /tmp, please deal with coredump file first."
  exit 1;
fi

if [ $# == 0 ] ; then
  json_path=$cur_path/config/os08a20_config.json
elif [ $# == 1 ] ; then
  json_path=$1
else
  echo "USAGE: $0 <configure file path>"
  echo " e.g.: $0 ./config/os08a20_config.json"
  exit 1;
fi

if [ ! -f $json_path ]; then
  echo "ERROR: Config file \"$json_path\" not found!"
  exit 1;
fi

#set -e
cd $cur_path

# load config
source ./config/${process}.conf

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib

# Open core dump
if [ $EnableCoreDump == 1 ] ; then
  ulimit -c unlimited
  echo /var/log/core-%e-%p-%t > /proc/sys/kernel/core_pattern
fi

# Open Mp4 recorder
if [ $Mp4Recorder == 1 ] ; then
  if [ -n "$Mp4SavedPath" ] ; then
    mkdir -p $Mp4SavedPath
    if [ ! -d "$Mp4SavedPath" ] ; then
      echo "ERROR: Create mp4 save path: \"$Mp4SavedPath\" failed!"
      exit 1;
    fi
  fi
  if [ $Mp4Saved2SDCard == 1 ] ; then
    umount $Mp4SavedPath
    mount /dev/mmcblk1p1 $Mp4SavedPath
    if [ 0 != $? ] ; then
      echo "ERROR: Mount TF card failed, mp4 recorder can not work!"
    else
      echo "Mp4 saved path: \"$Mp4SavedPath\"(TFCard)"
    fi
  else
    if [ ${#Mp4SavedPath} -ge 4 ] &&  [[ "${Mp4SavedPath:0:4}" == '/opt' ]] ; then
      echo "Mp4 saved path: \"$Mp4SavedPath\""
    else
      echo "Path: \"$Mp4SavedPath\" is invalid, make sure path set under \"/opt/*\""
      exit 1;
    fi
  fi
fi

# launch
if [ $RunInBackground == 1 ] ; then
  # Always export log to file if run in background
  nohup gdb --args ./${process} $json_path 2 $LogLevel $RTSPMaxFrmSize $WebFrmSizeRatio $ActiveDetect $DetectModel $PrintFPS $UseWebDeamon $Mp4Recorder $Mp4SavedPath $Mp4MaxFileNum $Mp4MaxFileSize 1 0 $DetectInputStrgy $EnableOSD $DetectAlgoType $DetectConfigPath $ActiveDetectTrack $ActiveDetectSearch $ActiveOD $ActiveMD $EnableAutoSleep $AutoSleepFrameNum $EnableEIS $EISDelayNum $EISCropW $EISCropH $EISEffectComp $RotationSet $MirrorSet $FlipSet &
else
  gdb --args ./${process} $json_path $LogTarget $LogLevel $RTSPMaxFrmSize $WebFrmSizeRatio $ActiveDetect $DetectModel $PrintFPS $UseWebDeamon $Mp4Recorder $Mp4SavedPath $Mp4MaxFileNum $Mp4MaxFileSize 1 0 $DetectInputStrgy $EnableOSD $DetectAlgoType $DetectConfigPath $ActiveDetectTrack $ActiveDetectSearch $ActiveOD $ActiveMD $EnableAutoSleep $AutoSleepFrameNum $EnableEIS $EISDelayNum $EISCropW $EISCropH $EISEffectComp $RotationSet $MirrorSet $FlipSet
fi
