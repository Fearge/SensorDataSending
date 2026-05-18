#!/usr/bin/env python3
"""
Simple RS485 master/parser for the reduced protocol.
Frame (7 bytes):
 0: SOF = 0xA5
 1: Type (0x01 = Balance, 0x10 = Sync)
 2: NodeID
 3: Payload LSB (balance int16 little-endian)
 4: Payload MSB
 5: CRC16 LSB (CRC over bytes [Type, NodeID, PayloadLSB, PayloadMSB], CCITT)
 6: CRC16 MSB

Usage:
  pip install pyserial
  python scripts/rs485_master.py --port COM5

Optional: --poll to periodically send a Sync (Type=0x10) broadcast frame
"""
import argparse
import serial
import struct
import time
import sys

SOF = 0xA5
TYPE_BALANCE = 0x01
TYPE_SYNC = 0x10


def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    crc = init
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


def parse_loop(ser: serial.Serial):
    while True:
        b = ser.read(1)
        if not b:
            continue
        if b[0] != SOF:
            # skip until SOF
            continue
        rest = ser.read(6)
        if len(rest) < 6:
            # incomplete, continue
            continue
        msg_type = rest[0]
        node_id = rest[1]
        payload_lsb = rest[2]
        payload_msb = rest[3]
        crc_l = rest[4]
        crc_h = rest[5]
        frame_crc = crc_l | (crc_h << 8)
        crc_in = bytes([msg_type, node_id, payload_lsb, payload_msb])
        computed = crc16_ccitt(crc_in)
        if computed != frame_crc:
            print(f"Bad CRC: got=0x{frame_crc:04X} calc=0x{computed:04X}  type=0x{msg_type:02X} node={node_id}")
            continue
        if msg_type == TYPE_BALANCE:
            # signed int16 little endian
            balance = struct.unpack('<h', bytes([payload_lsb, payload_msb]))[0]
            ts = time.strftime('%Y-%m-%d %H:%M:%S')
            print(f"{ts}  NODE {node_id:02d} BALANCE {balance}")
        elif msg_type == TYPE_SYNC:
            print(f"SYNC from node {node_id}")
        else:
            print(f"Unknown type 0x{msg_type:02X} from node {node_id}")


def send_sync(ser: serial.Serial, node_id: int = 0):
    # payload zeroed
    msg_type = TYPE_SYNC
    payload = bytes([0x00, 0x00])
    crc_in = bytes([msg_type, node_id]) + payload
    crc = crc16_ccitt(crc_in)
    frame = bytes([SOF, msg_type, node_id]) + payload + struct.pack('<H', crc)
    ser.write(frame)
    ser.flush()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', '-p', required=True, help='Serial port (COMx or /dev/ttyUSBx)')
    parser.add_argument('--baud', '-b', type=int, default=9600, help='Baudrate')
    parser.add_argument('--poll', action='store_true', help='Periodically send a Sync broadcast')
    parser.add_argument('--interval', type=float, default=2.0, help='Poll interval seconds')
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
    except Exception as e:
        print('Failed to open serial port:', e)
        sys.exit(1)

    print('Opened', args.port, 'baud', args.baud)

    try:
        last_poll = time.time()
        while True:
            if args.poll and (time.time() - last_poll) >= args.interval:
                # broadcast node_id 0
                send_sync(ser, node_id=0)
                last_poll = time.time()
            parse_loop(ser)
    except KeyboardInterrupt:
        print('\nExiting')
    finally:
        ser.close()


if __name__ == '__main__':
    main()
