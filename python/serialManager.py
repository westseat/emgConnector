import serial
import threading
import time
import serial.tools.list_ports

class HubManager(object):
    
    def __init__(self):
#        self.baudrate = baudrade
#        self.port = port
#        self.serial = None 
        self.listener = []

    def open(self):
        self.serial =serial.Serial()
        self.serial.baudrate = self.baudrate
        self.serial.port = self.port
        self.serial.open()

    def _open(self, port, baudrade = 115200):
        self.serial = serial.Serial()
        self.serial.baudrate = baudrade
        self.serial.port = port
        self.serial.open()
        return self.serial

    def addListener(self, listener):
        for i in self.listener:
            if i == listener:
                return
        self.listener.append(listener)

    def removeListener(self, listener):
        self.listener.remove(listener)

    def readData(self):
       self._run('/dev/ttyUSB0', 115200) 

    def _run(self, port, baudrade):
        ser = self._open(port, baudrade)
        print("open hub success")
        while True:
            s = ser.read(10)
            for listen in self.listener:
                listen.onMessage(s)

    def _monitor(self):
        while True:
            list_info = serial.tools.list_ports.comports()
            for dev in list_info:
                if(dev.device == '/dev/ttyUSB0'):
                    print("find avaliable hub")
                    thread = threading.Thread(target=self.readData, args=())
                    thread.daemon = True
                    thread.start()
                    return
            time.sleep(1)

    def start(self):
        thread = threading.Thread(target=self._monitor, args=())
        thread.daemon = True
        thread.start()    

class HubListener(object):
    def onMessage(self,data):
        pass
    
    def onSensorConnect(self,sensor):
        pass

    def onSensorDisconnect(self,sensor):
        pass

    def onHubConnect(self):
        pass

    def onHubDisConnect(self):
        pass

class emgHubListener(HubListener):
    def onMessage(self, data):
        for x in data:
            print(hex(x))

    def onHubConnect(self):
        print("Hub is connected ....")

if __name__ == '__main__':
    listener = emgHubListener()
    hub = HubManager()
    hub.addListener(listener)
    hub.start()
    while True:
        time.sleep(1)
