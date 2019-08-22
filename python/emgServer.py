from tornado import websocket, web, ioloop
import time
import threading
import hubManager
import parser

class emgParserListener(parser.emgHandler):
    def __init__(self):
        pass
    
    def onEmgData(self,data):
        loop.add_callback(SocketHandler.notify, data)
        #print(data)

class hubHandler(hubManager.HubListener):
    def __init__(self):
        self.__parser = parser.emgParser()
        self.__parser.registerListener(emgParserListener())


    def onMessage(self, data):
        #print(data)
        self.__parser.push(data)



class SocketHandler(websocket.WebSocketHandler):
    connection = set() 
    def open(self):
        self.connection.add(self)
        print("client connection: ", len(self.connection) )

    def on_close(self):
        self.connection.remove(self)
        print("client connection: ", len(self.connection))

    def check_origin(self, origin):
        return True
    
    @classmethod
    def notify(cls, msg):
        for clinet in cls.connection:
            clinet.write_message(msg)


def producer(loop, msg_callback):
    while True:
        time.sleep(0.01)
        data = "new data"
        loop.add_callback(msg_callback, data)

app = web.Application([(r'/', SocketHandler)])
loop = ioloop.IOLoop.current()

if __name__ == '__main__':
    #http_server = tornado.httpserver.HTTPServer(app)
    #http_server.listen(8876)
    hub = hubManager.HubManager()
    listenHub = hubHandler()
    hub.addListener(listenHub)
    hub.start()

    app.listen(8876)
#    tr = threading.Thread(target=producer,args=(loop, SocketHandler.notify))
#    tr.daemon = True
#    tr.start()
    ioloop.IOLoop.instance().start()