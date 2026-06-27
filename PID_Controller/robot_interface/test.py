import serial
import struct
import threading

ser = serial.Serial('COM5', 115200, timeout=1)

SOF1 = 0xAA
SOF2 = 0x55
PKT_TYPE_TELEMETRY = 0x01
PKT_TYPE_CMD = 0x02

def crc8(data):
    crc = 0x00
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = (crc << 1) ^ 0x07
            else:
                crc <<= 1
            crc &= 0xFF
    return crc

def send_command(kp_l, ki_l, kd_l, sp_l, kp_r, ki_r, kd_r, sp_r):
    payload = struct.pack('<ffffffff',
                          kp_l, ki_l, kd_l, sp_l,
                          kp_r, ki_r, kd_r, sp_r)
    crc_data = bytes([PKT_TYPE_CMD, len(payload)]) + payload
    crc = crc8(crc_data)
    frame = bytes([SOF1, SOF2, PKT_TYPE_CMD, len(payload)]) + payload + bytes([crc])
    ser.write(frame)
    print(f"[TX] Left:  kp={kp_l} ki={ki_l} kd={kd_l} sp={sp_l}")
    print(f"[TX] Right: kp={kp_r} ki={ki_r} kd={kd_r} sp={sp_r}")

def parse_frame(buf):
    if len(buf) < 40:
        return None

    (sp_l, vel_l, pos_l, out_l, enc_l,
     sp_r, vel_r, pos_r, out_r, enc_r) = struct.unpack('<ffffiffffi', buf[:40])
    #                                                    ↑ thêm 1 f nữa!

    return {
        'setpoint_left':    sp_l,
        'velocity_left':    vel_l,
        'position_left':    pos_l,
        'pid_output_left':  out_l,
        'encoder_raw_left': enc_l,

        'setpoint_right':    sp_r,
        'velocity_right':    vel_r,
        'position_right':    pos_r,
        'pid_output_right':  out_r,
        'encoder_raw_right': enc_r,
    }

def rx_thread():
    state = 'SOF1'
    pkt_type = 0
    pkt_len = 0
    payload_buf = []

    while True:
        byte = ser.read(1)
        if not byte:
            continue
        b = byte[0]

        if state == 'SOF1':
            if b == SOF1:
                state = 'SOF2'

        elif state == 'SOF2':
            if b == SOF2:
                state = 'TYPE'
            else:
                state = 'SOF1'

        elif state == 'TYPE':
            pkt_type = b
            state = 'LEN'

        elif state == 'LEN':
            pkt_len = b
            print(f"[DEBUG] pkt_len={pkt_len}")  # ← thêm vào đây
            payload_buf = []
            state = 'PAYLOAD'

        elif state == 'PAYLOAD':
            payload_buf.append(b)
            if len(payload_buf) >= pkt_len:
                state = 'CRC'

        elif state == 'CRC':
            crc_data = [pkt_type, pkt_len] + payload_buf
            expected = crc8(crc_data)
            if b == expected and pkt_type == PKT_TYPE_TELEMETRY:
                data = parse_frame(bytes(payload_buf))
                if data:
                    print(f"[RX] L: sp={data['setpoint_left']:.2f} "
                          f"vel={data['velocity_left']:.2f} "
                          f"out={data['pid_output_left']:.2f} "
                          f"enc={data['encoder_raw_left']} | "
                          f"R: sp={data['setpoint_right']:.2f} "
                          f"vel={data['velocity_right']:.2f} "
                          f"out={data['pid_output_right']:.2f} "
                          f"enc={data['encoder_raw_right']}")
            else:
                print(f"[CRC ERR] got={b:#x} expected={expected:#x}")
            state = 'SOF1'

# Start RX thread
t = threading.Thread(target=rx_thread, daemon=True)
t.start()

# Main thread
print("Commands:")
print("  send kp_l ki_l kd_l sp_l kp_r ki_r kd_r sp_r")
print("  Ví dụ: send 3.0 5.0 0.0 5.0 3.0 5.0 0.0 5.0")
print("  q: thoát")

while True:
    cmd = input()
    if cmd == 'q':
        break
    parts = cmd.split()
    if parts[0] == 'send' and len(parts) == 9:
        kp_l = float(parts[1])
        ki_l = float(parts[2])
        kd_l = float(parts[3])
        sp_l = float(parts[4])
        kp_r = float(parts[5])
        ki_r = float(parts[6])
        kd_r = float(parts[7])
        sp_r = float(parts[8])
        send_command(kp_l, ki_l, kd_l, sp_l, kp_r, ki_r, kd_r, sp_r)
    else:
        print("Sai cú pháp! Dùng: send kp_l ki_l kd_l sp_l kp_r ki_r kd_r sp_r")





"""
send 3.0 5.0 0.0 5.0 3.0 5.0 0.0 5.0   ← cả 2 motor sp=5
send 3.0 5.0 0.0 8.0 3.0 5.0 0.0 5.0   ← left sp=8, right sp=5
send 3.0 5.0 0.0 5.0 2.5 4.0 0.0 8.0   ← PID khác nhau + sp khác nhau
"""