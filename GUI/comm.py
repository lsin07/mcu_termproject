import serial

s_test = serial.Serial(port='COM7',timeout=0.5)

nosig_count = 0

while True:
    num = int.from_bytes(s_test.read(size=4))
    if (num == 0):
        print("No Signal")
        nosig_count += 1
        if nosig_count > 20:
            print("Connection timed out -- Check your connection.")
            break
    else:
        nosig_count = 0
        print(num)