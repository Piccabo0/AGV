import requests
import json
import base64
import os
import logging
import speech_recognition as sr
import serial 
import serial.tools.list_ports#文件名不要叫serial.py会冲突

#串口检测打印
def serial_detection():
    port_list = list(serial.tools.list_ports.comports())
    print(port_list)
    if len(port_list) == 0:
        print('无可用串口')
    else:
        for i in range(0,len(port_list)):
            print(port_list[i])

def serial_init():
    portx="COM7"
    bps=115200
    timex=None
    # 打开串口，并得到串口对象
    ser=serial.Serial(portx,bps,timeout=timex)
    # print("串口详情参数：", ser)
    #print(serial.Serial.isOpen(ser))#检测串口是否打开
    return ser

#获取token
def get_token():
    logging.info('Start to get the token！')
    #获取token
    baidu_server = "https://aip.baidubce.com/oauth/2.0/token?"
    grant_type = "client_credentials"
    client_id = "hpUHDiGevzaNQhYKosiGYZHe"#创建语音识别应用后的API Key
    client_secret = "SAGgyKmVpYqcQFzWmXOs4Vcd6eGnSKKK"#创建语音识别应用后的Secret Key

    #拼url
    url = f"{baidu_server}grant_type={grant_type}&client_id={client_id}&client_secret={client_secret}"
    res = requests.post(url)
    token = json.loads(res.text)["access_token"]
    return token

#根据api写的实时监测语音
def audio_baidu(filename):
    logging.info('Start Recognize Your Voice！')
    with open(filename, "rb") as f:
        speech = base64.b64encode(f.read()).decode('utf-8')
    size = os.path.getsize(filename)
    token = get_token()
    headers = {'Content-Type': 'application/json'}
    url = "http://vop.baidu.com/server_api"
    #从“https://ai.baidu.com/ai-doc/SPEECH/ek38lxj1u#%E8%AF%86%E5%88%AB%E6%A8%A1%E5%9E%8Bdev_pid%E5%8F%82%E6%95%B0”查看调用
    data = {
        "format": "wav",
        "rate": "16000",
        "dev_pid": "1537",# 1537 表示识别普通话，使用输入法模型。根据文档填写PID，选择语言及识别模型
        "speech": speech,
        "cuid": "baidu_workshop",#TEDxPY
        "len": size,
        "channel": 1,
        "token": token,
    }

    req = requests.post(url, json.dumps(data), headers)
    result = json.loads(req.text)
    # print(result)

    if result["err_msg"] == "success.":
        print(result['result'])
        return result['result']
    else:
        print("Content detection failed, It will exit the Voice Recognition!")
        return -1

def forward():
    word = "01160038000400000064000000640B15"
    ser.write((word + "\r").encode("utf-8"))  # 写数据
    print(word)


def back():
    word ="01160038000400010064000100644A15"
    ser.write((word + "\r").encode("utf-8"))  # 写数据
    print(word)

def brake():
    word ="01160038000400000000000000647ADD"
    ser.write((word + "\r").encode("utf-8"))  # 写数据
    print(word)    

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    wav_num = 0
    ser = serial_init()
    ser.write(("010600010000D80A" + "\r").encode("utf-8"))  # 配置小车为速度模式
    while (serial.Serial.isOpen(ser)):
        r = sr.Recognizer()
        #启用麦克风
        mic = sr.Microphone()
        logging.info('It is recording voice!')
        with mic as source:
            #降噪
            r.adjust_for_ambient_noise(source)
            audio = r.listen(source)
        with open(f"00{wav_num}.wav", "wb") as f:
            #将麦克风录到的声音保存为wav文件
            f.write(audio.get_wav_data(convert_rate=16000))
        logging.info('Recording ends, start to Recognize!')
        target = audio_baidu(f"00{wav_num}.wav")
        if target == -1:
            break
        else:
            wav_num += 1
            if ser.in_waiting:
                str1=ser.read(ser.in_waiting)
                sta =str(str1)
                print(sta)
            if target == ['前进。']:
                forward()
            elif target == ['后退。']:
                back()
            elif target == ['停止。']:
                brake()
