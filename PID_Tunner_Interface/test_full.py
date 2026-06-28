import serial
import struct
import threading

ser = serial.Serial('COM5', 115200, timeout=1)
fmt_tx = '<' + 'f'*4 + 'i' + 'f'*4 + 'i' + 'B' + 'f'*2 + 'f'*5
print(struct.calcsize(fmt_tx))  # phải = 69
# Verify sizes
print(struct.calcsize('<' + 'f'*14 + 'B' + 'ff'))  # cách chắc chắn nhất!

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
                 pos_kp_r=2.0, pos_ki_r=0.0, pos_kd_r=0.0,
                 use_diff_drive=0, linear_vel=0.0, angular_vel=0.0):

    fmt = '<' + 'f'*14 + 'B' + 'ff'  # 65 bytes
    payload = struct.pack(fmt,
                          kp_l, ki_l, kd_l, sp_l,
                          kp_r, ki_r, kd_r, sp_r,
                          pos_kp_l, pos_ki_l, pos_kd_l,
                          pos_kp_r, pos_ki_r, pos_kd_r,
                          use_diff_drive,
                          linear_vel, angular_vel)

    crc_data = bytes([PKT_TYPE_CMD, len(payload)]) + payload
    crc = crc8(crc_data)
    frame = bytes([SOF1, SOF2, PKT_TYPE_CMD, len(payload)]) + payload + bytes([crc])
    ser.write(frame)

    print(f"[TX] Left:  kp={kp_l} ki={ki_l} kd={kd_l} sp={sp_l}")
    print(f"[TX] Right: kp={kp_r} ki={ki_r} kd={kd_r} sp={sp_r}")
    print(f"[TX] DiffDrive: use={use_diff_drive} v={linear_vel} omega={angular_vel}")

fmt_tx = '<' + 'f'*4 + 'i' + 'f'*4 + 'i' + 'B' + 'f'*2 + 'f'*5

def parse_frame(buf):
    if len(buf) < 69:
        return None

    (sp_l, vel_l, pos_l, out_l, enc_l,
     sp_r, vel_r, pos_r, out_r, enc_r,
     mode, pos_sp_l, pos_sp_r,
     odom_x, odom_y, odom_theta,
     linear_vel, angular_vel) = struct.unpack(fmt_tx, buf[:69])

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
        'odom_x':             odom_x,
        'odom_y':             odom_y,
        'odom_theta':         odom_theta,
        'linear_vel':         linear_vel,
        'angular_vel':        angular_vel,
    }

def rx_thread():
    while True:
        b = ser.read(1)
        if not b or b[0] != 0xAA:
            continue
        b = ser.read(1)
        if not b or b[0] != 0x55:
            continue

        header = ser.read(2)
        if len(header) < 2:
            continue
        pkt_type = header[0]
        pkt_len = header[1]

        rest = ser.read(pkt_len + 1)
        if len(rest) < pkt_len + 1:
            continue

        payload = rest[:pkt_len]
        crc_got = rest[pkt_len]

        crc_data = bytes([pkt_type, pkt_len]) + payload
        crc_exp = crc8(crc_data)

        if crc_got == crc_exp and pkt_type == PKT_TYPE_TELEMETRY:
            data = parse_frame(payload)
            if data:
                print(f"[RX] ... | "
                                    f"x={data['odom_x']:.3f}m "
                                    f"y={data['odom_y']:.3f}m "
                                    f"θ={data['odom_theta']:.3f}rad | "
                                    f"v={data['linear_vel']:.3f}m/s "
                                    f"ω={data['angular_vel']:.3f}rad/s")
                # print(f"[RX] L: sp={data['setpoint_left']:.2f} "
                #       f"vel={data['velocity_left']:.2f} "
                #       f"pos={data['position_left']:.2f} "
                #       f"out={data['pid_output_left']:.2f} | "
                #       f"R: sp={data['setpoint_right']:.2f} "
                #       f"vel={data['velocity_right']:.2f} "
                #       f"pos={data['position_right']:.2f} "
                #       f"out={data['pid_output_right']:.2f} | "
                #       f"mode={'POS' if data['mode'] else 'VEL'} "
                #       f"pos_sp_L={data['pos_setpoint_left']:.2f} "
                #       f"pos_sp_R={data['pos_setpoint_right']:.2f}")
        else:
            print(f"[CRC ERR] got={crc_got:#x} expected={crc_exp:#x}")

# Start RX thread
t = threading.Thread(target=rx_thread, daemon=True)
t.start()

# Main thread
print("Commands:")
print("  send kp_l ki_l kd_l sp_l kp_r ki_r kd_r sp_r [pos_kp_l pos_ki_l pos_kd_l pos_kp_r pos_ki_r pos_kd_r] [use_diff v omega]")
print("  Ví dụ rad/s: send 3.0 5.0 0.0 5.0 3.0 5.0 0.0 5.0")
print("  Ví dụ diff:  send 3.0 5.0 0.0 0.0 3.0 5.0 0.0 0.0 2.0 0.0 0.0 2.0 0.0 0.0 1 0.3 0.0")
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

        use_diff  = int(float(parts[15]))   if len(parts) > 15 else 0
        linear    = float(parts[16]) if len(parts) > 16 else 0.0
        angular   = float(parts[17]) if len(parts) > 17 else 0.0

        send_command(kp_l, ki_l, kd_l, sp_l,
                     kp_r, ki_r, kd_r, sp_r,
                     pos_kp_l, pos_ki_l, pos_kd_l,
                     pos_kp_r, pos_ki_r, pos_kd_r,
                     use_diff, linear, angular)
    else:
        print("Sai cú pháp!")


# send 3.0 5.0 0.0 5.0 3.0 5.0 0.0 5.0
# send 3.0 5.0 0.0 0.0 3.0 5.0 0.0 0.0 2.0 0.0 0.0 2.0 0.0 0.0 1 0.3 0.0


# Quay tại chỗ
# send 3.0 5.0 0.0 0.0 3.0 5.0 0.0 0.0 2.0 0.0 0.0 2.0 0.0 0.0 1 0.0 1.0

# Rẽ trái
# send 3.0 5.0 0.0 0.0 3.0 5.0 0.0 0.0 2.0 0.0 0.0 2.0 0.0 0.0 1 0.3 0.5

# Dừng
# send 3.0 5.0 0.0 0.0 3.0 5.0 0.0 0.0 2.0 0.0 0.0 2.0 0.0 0.0 1 0.0 0.0