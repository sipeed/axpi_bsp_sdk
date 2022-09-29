test_flag = False

try:
    from gpiod import chip, line, line_request
    config = None # rpi is default value A 0
    def gpio(gpio_line=0, gpio_bank="a", gpio_chip=0, line_mode = line_request.DIRECTION_OUTPUT):
        global config
        if config != None and gpio_line in config:
            gpio_bank, gpio_chip = config[gpio_line]
        l, c = [32 * (ord(gpio_bank.lower()[0]) - ord('a')) + gpio_line, chip("gpiochip%d" % gpio_chip)]
        tmp = c.get_line(l)
        cfg = line_request() # led.active_state == line.ACTIVE_LOW
        cfg.request_type = line_mode # line.DIRECTION_INPUT
        tmp.request(cfg)
        tmp.source = "GPIO chip %s bank %s line %d" % (gpio_chip, gpio_bank, gpio_line)
        return tmp
    def load(cfg=None):
        global config
        config = cfg
except ModuleNotFoundError as e:
    pass

key = gpio(30, gpio_chip=0, line_mode = line_request.DIRECTION_INPUT)
led0 = gpio(4, gpio_chip=2, line_mode = line_request.DIRECTION_OUTPUT)
led1 = gpio(5, gpio_chip=2, line_mode = line_request.DIRECTION_OUTPUT)

import time
import ifcfg
import os

def check_ifconfig():
    result = []
    for name, interface in ifcfg.interfaces().items():
        if name in ['eth0', 'wlan0'] and interface['inet']:
            result.append(name)
    return result

try:
    if (0 == key.get_value()):
        os.system("export LD_LIBRARY_PATH=/opt/lib:LD_LIBRARY_PATH && /opt/bin/sample_vin_vo -c 2 -e 1 -s 0 -v dsi0@800x1280@45 &")
        led1.set_value(1)
        while True:
            led0.set_value(1)
            time.sleep(0.2)
            led0.set_value(0)
            time.sleep(0.2)
            tmp = check_ifconfig()
            if len(tmp) > 1:
                led0.set_value(0)
                led1.set_value(0)
                test_flag = True
                break
        while (0 == key.get_value()):
            time.sleep(0.2)
        os.system("aplay /home/examples/boot.wav")
        led0.set_value(1)
        led1.set_value(1)
        import pyaudio
        chunk = 1024      # Each chunk will consist of 1024 samples
        sample_format = pyaudio.paInt16      # 16 bits per sample
        channels = 2      # Number of audio channels
        fs = 44100        # Record at 44100 samples per second
        p = pyaudio.PyAudio()
        stream = p.open(format=sample_format,
                        channels = channels,
                        rate = fs,
                        frames_per_buffer = chunk,
                        input = True, output = True)
        while (1 == key.get_value()):
            data = stream.read(chunk, exception_on_overflow = False)
            stream.write(data)
        while (0 == key.get_value()):
            time.sleep(0.2)
        os.system('killall sample_vin_vo')
        os.system('killall sample_vin_vo')
        # Stop and close the Stream and PyAudio
        stream.stop_stream()
        stream.close()
        p.terminate()
except Exception as e:
    print(e)
finally:
    if test_flag:
        led0.set_value(0)
        led1.set_value(0)

'''

import pyaudio

try:
    chunk = 1024      # Each chunk will consist of 1024 samples
    sample_format = pyaudio.paInt16      # 16 bits per sample
    channels = 2      # Number of audio channels
    fs = 44100        # Record at 44100 samples per second
    time_in_seconds = 3
    p = pyaudio.PyAudio()
    stream = p.open(format=sample_format,
                    channels = channels,
                    rate = fs,
                    frames_per_buffer = chunk,
                    input = True, output = True)

    stream.write(stream.read(chunk))

    # # Store data in chunks for 3 seconds
    # for i in range(0, int(fs / chunk * time_in_seconds)):
    #     data = stream.read(chunk)
    #     stream.write(data)

finally:
    # Stop and close the Stream and PyAudio
    stream.stop_stream()
    stream.close()
    p.terminate()
'''

