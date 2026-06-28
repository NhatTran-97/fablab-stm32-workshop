import serial
import struct
import threading

ser = serial.Serial('COM5', 115200, timeout=1)
#print(struct.calcsize('<ffffffffffffBff'))



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

def send_command(kp_l, ki_l, kd_l, sp_l,
                 kp_r, ki_r, kd_r, sp_r,
                 pos_kp_l=2.0, pos_ki_l=0.0, pos_kd_l=0.0,
                 pos_kp_r=2.0, pos_ki_r=0.0, pos_kd_r=0.0):

    payload = struct.pack('<ffffffffffffff',
                          kp_l, ki_l, kd_l, sp_l,
                          kp_r, ki_r, kd_r, sp_r,
                          pos_kp_l, pos_ki_l, pos_kd_l,
                          pos_kp_r, pos_ki_r, pos_kd_r)

    crc_data = bytes([PKT_TYPE_CMD, len(payload)]) + payload
    crc = crc8(crc_data)
    frame = bytes([SOF1, SOF2, PKT_TYPE_CMD, len(payload)]) + payload + bytes([crc])
    ser.write(frame)

    print(f"[TX] Left:  kp={kp_l} ki={ki_l} kd={kd_l} sp={sp_l}")
    print(f"[TX] Right: kp={kp_r} ki={ki_r} kd={kd_r} sp={sp_r}")
    print(f"[TX] Pos PID L: kp={pos_kp_l} ki={pos_ki_l} kd={pos_kd_l}")
    print(f"[TX] Pos PID R: kp={pos_kp_r} ki={pos_ki_r} kd={pos_kd_r}")

def parse_frame(buf):
    if len(buf) < 49:
        return None

    (sp_l, vel_l, pos_l, out_l, enc_l,
     sp_r, vel_r, pos_r, out_r, enc_r,
     mode, pos_sp_l, pos_sp_r) = struct.unpack('<ffffiffffiBff', buf[:49])

    return {
        'setpoint_left':      sp_l,
        'velocity_left':      vel_l,
        'position_left':      pos_l,
        'pid_output_left':    out_l,
        'encoder_raw_left':   enc_l,
        'setpoint_right':     sp_r,
        'velocity_right':     vel_r,
        'position_right':     pos_r,
        'pid_output_right':   out_r,
        'encoder_raw_right':  enc_r,
        'mode':               mode,
        'pos_setpoint_left':  pos_sp_l,
        'pos_setpoint_right': pos_sp_r,
    }


def rx_thread():
    while True:
        # Tìm AA 55 trước
        b = ser.read(1)
        if not b or b[0] != 0xAA:
            continue
        b = ser.read(1)
        if not b or b[0] != 0x55:
            continue

        # Đọc type + len
        header = ser.read(2)
        if len(header) < 2:
            continue
        pkt_type = header[0]
        pkt_len = header[1]

        # Đọc payload + CRC
        rest = ser.read(pkt_len + 1)
        if len(rest) < pkt_len + 1:
            continue

        payload = rest[:pkt_len]
        crc_got = rest[pkt_len]

        # Verify CRC
        crc_data = bytes([pkt_type, pkt_len]) + payload
        crc_exp = crc8(crc_data)

        print(f"[DEBUG] type={pkt_type:#x} len={pkt_len} crc_got={crc_got:#x} crc_exp={crc_exp:#x}")

        if crc_got == crc_exp and pkt_type == PKT_TYPE_TELEMETRY:
            data = parse_frame(payload)
            if data:
                print(f"[RX] L: sp={data['setpoint_left']:.2f} "
                      f"vel={data['velocity_left']:.2f} "
                      f"pos={data['position_left']:.2f} "
                      f"out={data['pid_output_left']:.2f} | "
                      f"R: sp={data['setpoint_right']:.2f} "
                      f"vel={data['velocity_right']:.2f} "
                      f"pos={data['position_right']:.2f} "
                      f"out={data['pid_output_right']:.2f} | "
                      f"mode={'POS' if data['mode'] else 'VEL'} "
                      f"pos_sp_L={data['pos_setpoint_left']:.2f} "
                      f"pos_sp_R={data['pos_setpoint_right']:.2f}")
        else:
            print(f"[CRC ERR] got={crc_got:#x} expected={crc_exp:#x}")

# Start RX thread
t = threading.Thread(target=rx_thread, daemon=True)
t.start()

# Main thread
print("Commands:")
print("  send kp_l ki_l kd_l sp_l kp_r ki_r kd_r sp_r [pos_kp_l pos_ki_l pos_kd_l pos_kp_r pos_ki_r pos_kd_r]")
print("  Ví dụ: send 3.0 5.0 0.0 5.0 3.0 5.0 0.0 5.0")
print("  q: thoát")

while True:
    cmd = input()
    if cmd == 'q':
        break
    parts = cmd.split()
    if parts[0] == 'send' and len(parts) >= 9:
        kp_l = float(parts[1])
        ki_l = float(parts[2])
        kd_l = float(parts[3])
        sp_l = float(parts[4])
        kp_r = float(parts[5])
        ki_r = float(parts[6])
        kd_r = float(parts[7])
        sp_r = float(parts[8])

        pos_kp_l = float(parts[9])  if len(parts) > 9  else 2.0
        pos_ki_l = float(parts[10]) if len(parts) > 10 else 0.0
        pos_kd_l = float(parts[11]) if len(parts) > 11 else 0.0
        pos_kp_r = float(parts[12]) if len(parts) > 12 else 2.0
        pos_ki_r = float(parts[13]) if len(parts) > 13 else 0.0
        pos_kd_r = float(parts[14]) if len(parts) > 14 else 0.0

        send_command(kp_l, ki_l, kd_l, sp_l,
                     kp_r, ki_r, kd_r, sp_r,
                     pos_kp_l, pos_ki_l, pos_kd_l,
                     pos_kp_r, pos_ki_r, pos_kd_r)
    else:
        print("Sai cú pháp! Dùng: send kp_l ki_l kd_l sp_l kp_r ki_r kd_r sp_r [pos_kp_l pos_ki_l pos_kd_l pos_kp_r pos_ki_r pos_kd_r]")






