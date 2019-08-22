import enum

class emgPraserStatus(enum.Enum):
    MAG_NUM_1 = 1
    MAG_NUM_2 = 2
    FIRST_FLAG = 3
    SECOND_FLAG = 4
    THIRD_FLAG = 5
    FORTH_FLAG = 6
    FIFITH_FLAG = 7
    SIXTH_FLAG = 8
    SEVENTH_FLAG = 9
    EIGHTTH_FLAG = 10
    UNDEFINED = 11

class emgParser(object):
    def __init__(self):
        self.__listener = set()
        self.__status = emgPraserStatus.UNDEFINED 
        self.__value = {}
        self.__parserData= ""

    def push(self, data):
        ##print("data from prser")
        for value in data:
            #print(value)
            if self.__status == emgPraserStatus.UNDEFINED:
                if value != 0x0d:
                    continue
                self.__parserData = ""
                self.__status = emgPraserStatus.MAG_NUM_1
                #print("MAG_NUM_1 status")
                #print(hex(value))
            elif self.__status == emgPraserStatus.MAG_NUM_1:
                #print("Try to find status2")
                #print(hex(value))
                if value != 0x0a:
                    self.__status = emgPraserStatus.UNDEFINED
                    continue
                self.__status = emgPraserStatus.MAG_NUM_2
                #print("MAG_NUM_2 STATUS")
            elif self.__status == emgPraserStatus.MAG_NUM_2:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #self.__value['ch1Average'] = int(self.__parserData)
                #print("Firt_FLag:", self.__parserData)
                self.__parserData = ""
                self.__status =emgPraserStatus.FIRST_FLAG
                #print("FITRST_FLAG")
            elif self.__status == emgPraserStatus.FIRST_FLAG:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print("second_flag:",self.__parserData)
                self.__value['ch1Value'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.SECOND_FLAG
                #print("SECOND_FLAG")
            elif self.__status == emgPraserStatus.SECOND_FLAG:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print('third_flag:',self.__parserData)
                self.__value['ch1Power'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.THIRD_FLAG
                #print("THIRD_FLAG")
            elif self.__status == emgPraserStatus.THIRD_FLAG:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print('forth_flag:',self.__parserData)
                self.__value['ch1Strength'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.FORTH_FLAG
            elif self.__status == emgPraserStatus.FORTH_FLAG:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print('fifth_flag:',self.__parserData)
                self.__value['ch2Average'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.FIFITH_FLAG
            elif self.__status == emgPraserStatus.FIFITH_FLAG:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print('sixth_flag:',self.__parserData)
                self.__value['ch2Value'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.SIXTH_FLAG
            elif self.__status == emgPraserStatus.SIXTH_FLAG:
                if value != 0x2c:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print('seventh_flag:',self.__parserData)
                self.__value['ch2Power'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.SEVENTH_FLAG
            elif self.__status == emgPraserStatus.SEVENTH_FLAG:
                if value != 0x0d:
                    self.__parserData = self.__parserData + chr(value)
                    continue
                #print('eigthh_flag:',self.__parserData)
                self.__value['ch2Strength'] = int(self.__parserData)
                self.__parserData = ""
                self.__status = emgPraserStatus.MAG_NUM_1
                for listener in self.__listener:
                    listener.onEmgData(self.__value)

    def registerListener(self, listener):
       self.__listener.add(listener) 

    def removeListener(self, listener):
        self.__listener.remove(listener)

    def reset(self):
        self.__status = emgPraserStatus.UNDEFINED
        self.__parserData = ""


class emgHandler(object):
    def __init__(self):
        pass

    def onEmgData(self,data):
        pass