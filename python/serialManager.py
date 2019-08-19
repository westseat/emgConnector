import serial

class serialManager(object):
    
    def __init__(self, baudrade, port):
        self.baudrate = baudrade
        self.port = port
        self.serial = None 
        self.listener = []

    def open(self):
        self.serial =serial.Serial()
        self.serial.baudrate = self.baudrate
        self.serial.port = self.port
        self.serial.open()

    def addListener(self, listener):
        for i in self.listener:
            if i == listener:
                return
        self.listener.append(listener)

    def removeListener(self, listener):
        self.listener.remove(listener)

    def run(self):
        pass

    def runTest(self):
        while True:
            s = self.serial.read(10)
            for listen in self.listener:
                listen.onMessage(s)

class EmgListener(object):
    
    def __init__(self):
        pass

    def onMessage(slef, data):
        for x in data:
            print(hex(x))

if __name__ == '__main__':
    s = serialManager(115200, '/dev/ttyUSB0')
    listener = EmgListener()
    s.addListener(listener)
    s.open()
    s.runTest()