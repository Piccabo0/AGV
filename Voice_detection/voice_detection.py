import requests
import json
import base64
import os
import logging
import speech_recognition as sr

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

    if result["err_msg"] == "success.":
        print(result['result'])
        return result['result']
    else:
        print("Content detection failed, It will exit the Voice Recognition!")
        return -1


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    wav_num = 0
    while True:
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
        wav_num += 1
