#!/usr/bin/env python3
"""
Simple RS485 master for sequential polling.

Frame (7 bytes):
 0: SOF = 0xA5
 1: Type (0x01 = Balance, 0x10 = Poll)
 2: NodeID
 3: Payload LSB
 4: Payload MSB
 5: CRC16 LSB (CRC over bytes [Type, NodeID, PayloadLSB, PayloadMSB], CCITT)
 6: CRC16 MSB

Usage:
    pip install pyserial
    python scripts/rs485_master.py --port COM5

By default the master polls NodeIDs 1, 2, 3, 4 in order and prints the first
balance frame returned by each node.
"""
import argparse
import serial
import time
import sys
import statistics

SOF = 0xA5
TYPE_BALANCE = 0x01
TYPE_POLL = 0x10
DEFAULT_NODE_IDS = (1, 2, 3, 4)


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


def build_poll_frame(node_id: int) -> bytes:
    msg_type = TYPE_POLL
    payload = bytes([0x00, 0x00])
    crc_in = bytes([msg_type, node_id]) + payload
    crc = crc16_ccitt(crc_in)
    return bytes([SOF, msg_type, node_id]) + payload + crc.to_bytes(2, "little")


def measure_latency(ser: serial.Serial, node_id: int, samples: int = 100, timeout_s: float = 0.05):
    """Measure round-trip latency (ms) for a node by sending poll frames.
    Runs `samples` requests and prints timing statistics.
    Returns stats dict or None if no successful responses.
    """
    deltas_ms = []
    timeouts = 0
    for i in range(samples):
        t0 = time.perf_counter_ns()
        ser.write(build_poll_frame(node_id))
        ser.flush()
        frame = read_frame(ser, timeout_s=timeout_s)
        t1 = time.perf_counter_ns()
        if frame is None:
            timeouts += 1
        else:
            delta_ms = (t1 - t0) / 1e6
            deltas_ms.append(delta_ms)
        time.sleep(0.01)

    if not deltas_ms:
        print(f"NODE {node_id:02d}: no successful responses ({timeouts} timeouts)")
        return None

    deltas_ms.sort()
    count = len(deltas_ms)
    stats = {
        'samples_requested': samples,
        'responses': count,
        'timeouts': timeouts,
        'min_ms': deltas_ms[0],
        'median_ms': statistics.median(deltas_ms),
        'p90_ms': deltas_ms[int(0.9 * (count - 1))],
        'max_ms': deltas_ms[-1],
        'mean_ms': statistics.mean(deltas_ms),
    }

    print(f"NODE {node_id:02d} latency: {stats['responses']}/{stats['samples_requested']} responses, "
          f"min {stats['min_ms']:.3f} ms, med {stats['median_ms']:.3f} ms, "
          f"p90 {stats['p90_ms']:.3f} ms, max {stats['max_ms']:.3f} ms, mean {stats['mean_ms']:.3f} ms")
    return stats


def read_frame(ser: serial.Serial, timeout_s: float = 0.05):
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        b = ser.read(1)
        if not b:
            continue
        if b[0] != SOF:
            continue
        rest = ser.read(6)
        if len(rest) < 6:
            continue
        msg_type = rest[0]
        node_id = rest[1]
        payload_lsb = rest[2]
        payload_msb = rest[3]
        frame_crc = rest[4] | (rest[5] << 8)
        crc_in = bytes([msg_type, node_id, payload_lsb, payload_msb])
        computed = crc16_ccitt(crc_in)
        if computed != frame_crc:
            continue
        return msg_type, node_id, payload_lsb, payload_msb
    return None





def poll_node(ser: serial.Serial, node_id: int, timeout_s: float = 0.05):
    ser.write(build_poll_frame(node_id))
    ser.flush()

    frame = read_frame(ser, timeout_s=timeout_s)
    if frame is None:
        print(f"NODE {node_id:02d}: no response")
        return

    msg_type, rx_node_id, payload_lsb, payload_msb = frame
    if msg_type == TYPE_BALANCE and rx_node_id == node_id:
        value = int.from_bytes(bytes([payload_lsb, payload_msb]), byteorder="little", signed=True)
        ts = time.strftime("%Y-%m-%d %H:%M:%S")
        print(f"{ts}  NODE {rx_node_id:02d} BALANCE {value}")
    else:
        print(f"NODE {node_id:02d}: unexpected frame type 0x{msg_type:02X} from node {rx_node_id}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', '-p', required=True, help='Serial port (COMx or /dev/ttyUSBx)')
    parser.add_argument('--baud', '-b', type=int, default=9600, help='Baudrate')
    parser.add_argument('--nodes', nargs='*', type=int, default=list(DEFAULT_NODE_IDS), help='NodeIDs to poll in order')
    parser.add_argument('--timeout', type=float, default=0.05, help='Response timeout per node in seconds')
    parser.add_argument('--measure-latency', action='store_true', help='Run latency measurement per node then exit')
    parser.add_argument('--latency-samples', type=int, default=100, help='Number of latency samples per node')
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
    except Exception as e:
        print('Failed to open serial port:', e)
        sys.exit(1)

    print('Opened', args.port, 'baud', args.baud)

    try:
        while True:
            for node_id in args.nodes:
                if args.measure_latency:
                    measure_latency(ser, node_id, samples=args.latency_samples, timeout_s=args.timeout)
                else:
                    poll_node(ser, node_id, timeout_s=args.timeout)
            if args.measure_latency:
                break
    except KeyboardInterrupt:
        print('\nExiting')
    finally:
        ser.close()


if __name__ == '__main__':
    main()
