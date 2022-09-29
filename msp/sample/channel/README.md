sample_channel

1）功能说明：
  (1) 该demo是级联主从sdio通信演示demo，分主(master)从(slave)两部分。
  (2) 主从两端均使用可执行文件sample_channel，并通过命令行参数-t区分是master还是slave侧程序。
  (3) -k参数表示测试case号，目前有001/002/003/004四种：
      - 001 => 小包写/回读/校验(单线程，每包小于512字节)
      - 002 => 大包写/回读/校验(单线程，每包大于64KB)
      - 003 => 大小包混合做测试(多线程)
      - 004 => 主片上传或下载文件

2）使用示例：
  (1) 主片侧(master)
  usage: ./sample_channel [options] ...
  options:
    -t, --type    channel type (string [=m])              (m: master)
    -k, --task    task type (string [=001])               (测试case号, 仅master端有效, 有001/002/003/004四种)
    -d, --dev     device name (string [=/dev/sdio_host])  (master是/dev/sdio_host)
    -w, --wait    wait interval (unsigned int [=1])       (测试等待间隔(秒)，与--loop连用，仅master端有效)
    -c  --count   thread count (unsigned int [=2])        (目前仅对测试case 003有效，为配置的线程数)
    -v, --log     log level (unsigned int [=4])           (日志等级，对应AX_CHANNEL_LOG_LEVEL_E)
        --loop    loop test                               (是否loop测试)
    -?, --help    print this message

  extra options:
    -l, --download download file from slave (string [=])  (主片从从片下载文件，主片需指定待保存的完整文件路径(路径+文件名))
    -u, --upload   upload file to slave (string [=])      (主片上传文件到从片，需指定主片目录下的完整文件路径(路径+文件名))

  (2) 从片侧(slave)
  usage: ./sample_channel [options] ...
  options:
    -t, --type    type (string [=s])                      (s: slave)
    -d, --dev     device name (string [=/dev/sdio_slave]) (slave是/dev/sdio_slave)
    -v, --log     log level (unsigned int [=4])           (日志等级，对应AX_CHANNEL_LOG_LEVEL_E)
        --notify  slave notify data                       (是否开启slave主动上报)
    -?, --help    print this message

  extra options:
    -l, --download download file from slave (string [=])  (响应主片从从片下载文件，需指定从片目录下的完整文件路径(路径+文件名))
    -u, --upload   upload file to slave (string [=])      (响应主片上传文件到从片，从片需指定待保存的完整文件路径(路径+文件名))

3）示例运行结果：
  (1) 主片侧(master，运行测试case 001)
  ./sample_channel -t m -k 001 -d /dev/sdio_host -w 1 -v 6 --loop
  ********************************************************
  * SAMPLE BUILD: Dec  1 2021 12:37:35 *
  ********************************************************
  master cmdline params: task[001] dev[/dev/sdio_host] bLoop[1] wait[1] dbg level[6]
  [2021-12-01 12:49:26:593] RegNotifyCallback:461 add callback addr: 0x12d51
  [2021-12-01 12:49:26:593] SetDrvObj: SDIO = 1
  [2021-12-01 12:49:26:594] open /dev/sdio_host success
  [2021-12-01 12:49:26:594] Wakeup
  [2021-12-01 12:49:26:594] ioctl (PREISP_SDIO_CHECK) success
  [2021-12-01 12:49:26:594] device is plugin.
  [2021-12-01 12:49:26:598] ioctl (PREISP_SDIO_INIT) success
  ...

  (2) 从片侧(slave)
  ./sample_channel -t s -d /dev/sdio_slave -v 6
  ********************************************************
  * SAMPLE BUILD: Dec  1 2021 12:37:35 *
  ********************************************************
  slave cmdline params: dev[/dev/sdio_slave] bNotify[0] dbg level[6]
  [1970-01-01 08:06:09:660] RegMsgCallback:479 add callback addr: 0x12d95
  [1970-01-01 08:06:09:660] open /dev/sdio_slave success
  [1970-01-01 08:06:09:670] ioctl (AX_SDIO_SLAVE_GPIO_SET) success
  [1970-01-01 08:06:09:670] init channel success
  [1970-01-01 08:06:09:670] api instance count: 1
  ...

4）注意事项：
  (1) 目前仅支持在AX620级联板上测试，在主片和从片上的/opt/bin/目录下均有sample_channel可执行文件。
      运行测试时，需要先运行从片上的sample_channel(-t参数指定s)，再运行主片上的(-t参数指定m)。
