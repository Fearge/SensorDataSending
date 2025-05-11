import sliplib
ser = serial.Serial('/dev/ttyUB0', 9600)
while True:
    s = ser.read(10000)
    print(s)