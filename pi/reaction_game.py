import serial
import RPi.GPIO as GPIO
import time

LED_PINS = {1: 17, 2: 18, 3: 22, 4: 23, 5: 24}
BUTTON_PINS = {1: 25, 2: 27}

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

for pin in LED_PINS.values():
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

for pin in BUTTON_PINS.values():
    GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

ser = serial.Serial('/dev/serial0', 9600, timeout=0.05)

print("Reaction Game gestartet, warte auf STM32...")

try:
    while True:
        if ser.in_waiting:
            line = ser.readline().decode(errors='replace').strip()
            if line:
                print(f"STM32: {line}")

                if len(line) == 2 and line[0] == 'L':
                    led_num = int(line[1])
                    if led_num in LED_PINS:
                        GPIO.output(LED_PINS[led_num], GPIO.HIGH)

                elif len(line) == 2 and line[0] == 'l':
                    led_num = int(line[1])
                    if led_num in LED_PINS:
                        GPIO.output(LED_PINS[led_num], GPIO.LOW)

        for player_num, pin in BUTTON_PINS.items():
            if GPIO.input(pin) == GPIO.LOW:
                cmd = f"B{player_num}\n"
                ser.write(cmd.encode())
                print(f"-> Sende: {cmd.strip()}")
                while GPIO.input(pin) == GPIO.LOW:
                    time.sleep(0.01)

        time.sleep(0.005)

except KeyboardInterrupt:
    print("Beende...")
finally:
    GPIO.cleanup()
    ser.close()
