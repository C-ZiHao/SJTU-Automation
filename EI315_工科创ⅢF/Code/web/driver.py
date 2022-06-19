import serial
class driver:
    def __init__(self,portx = "/dev/ttyAMA0",bps = 57600):
        self.portx = portx
        self.bps = bps
        timex = 0.01
        self.ser = serial.Serial(portx,bps,timeout = timex,parity='N',stopbits = 1,bytesize=8)
        
    def set_speed(self,x,y):
        self.ser.write(("speed:%d,%d\r\n"%(x,y)).encode())
        return self.ser.read(20).decode()
    def read_battery(self):
        self.ser.write(("battery?\r\n").encode())
        return self.ser.read(20).decode()
    def get_sensor(self):
        self.ser.write(("ob_sensor?\r\n").encode())
        return self.ser.read(20).decode()

