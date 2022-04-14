import serial #导入模块
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
    
import threading
import time
 
import serial #导入模块
def aaa():
    try:
      portx="COM7"
      bps=115200
      timex=None    #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
      # 打开串口，并得到串口对象
      ser=serial.Serial(portx,bps,timeout=timex)
      print("串口详情参数：", ser)
      #print(ser.port)#获取到当前打开的串口名
      #print(ser.baudrate)#获取波特率
      ser.write('aaaaaaa\r'.encode("utf-8")) #发送指令
      def aa():
          a = input()
          #ser.write(("\r").encode("utf-8"))  # 写数据
          ser.write((a + "\r").encode("utf-8"))  # 写数据
 
      while True:
             if ser.in_waiting:
                 str1=ser.read(ser.in_waiting)
                 sta =str(str1)
                 if(sta=="exit"):#退出标志
                    break
                 else:
                    print(sta)
                    t2 = threading.Thread(target=aa)
                    # t1.start()
                    t2.start()
 
      ser.close()#关闭串口
    except Exception as e:
        print("---异常---：",e)



serial_detection()
aaa()
