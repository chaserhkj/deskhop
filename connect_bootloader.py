from prompt_toolkit import (
    print_formatted_text as print,
    PromptSession
    )
from prompt_toolkit.formatted_text import FormattedText
from prompt_toolkit.shortcuts import ProgressBar
import hid
import traceback
from binascii import hexlify, crc32
import time, os

VID=0x2e8a
PID=0x107c

BUFFER_SIZE = 1048
MAX_REPORT_SIZE = 64
# One byte for message type
MAX_PAYLOAD_SIZE = MAX_REPORT_SIZE - 1
# For reads and writes, first 2 bytes are read/write offset
MAX_WRITE_SIZE = MAX_PAYLOAD_SIZE - 2
MAX_READ_SIZE = MAX_PAYLOAD_SIZE - 2

TYPE_WRITE = b'\x02'
TYPE_SEND = b'\x03'
TYPE_READ = b'\x04'
TYPE_STOP = b'\x05'

DESC_GENERIC = b'\x04'
RESP_OK = b'\x01'
RESP_OOB = b'\x02'

IMG_OFFSET = 0x10004000
SECTOR = 1024
BLOCK = SECTOR*4

print_italic = lambda x: print(FormattedText([("italic", x)]))
print_red = lambda x: print(FormattedText([("#ff0000 bold", x)]))
print_bold = lambda x: print(FormattedText([("bold", x)]))

def to_opcode(code):
    assert(isinstance(code, str))
    assert(len(code) == 4)
    code = ord(code[0]) | (ord(code[1]) << 8) | (ord(code[2]) << 16) | (ord(code[3]) << 24)
    code = code.to_bytes(4, "little", signed=False)
    return code

def chunk(b, size):
    cur = 0
    while cur < len(b):
        yield b[cur:cur+size]
        cur += size

def write(dev, msg, chunk_size=MAX_WRITE_SIZE):
    assert(len(msg) < BUFFER_SIZE)
    assert(isinstance(msg, bytes))
    offset = 0xffff
    checksum = 0
    t = len(msg)//chunk_size + 1
    for n, c in enumerate(chunk(msg, chunk_size)):
        for i in c:
            checksum = checksum^i
        data = b"\x00" # Dummy descriptor header
        data += TYPE_WRITE
        data += offset.to_bytes(2, "little", signed=False)
        data += c
        dev.write(data)
        if (resp := wait_reply(dev)) != RESP_OK:
            raise Exception( f"Write error, resp: {hexlify(resp).decode()}" )
        addr_display = "RST0x0" if offset == 0xffff else "0x{:04X}".format(offset)
        # print_italic(f"BUF {addr_display}: Wrote {len(c)} ({n+1}/{t})")
        if offset == 0xffff:
            offset = len(c)
        else:
            offset += len(c)
    return checksum

def read(dev):
    result = b''
    while True:
        data = b"\x00" # Dummy descriptor header
        data += TYPE_READ
        data += len(result).to_bytes(2, "little", signed=False)
        dev.write(data)

        resp = wait_reply(dev)
        if resp[:1] == RESP_OOB:
            break
        if resp[:1] == RESP_OK:
            result += resp[1:]
            print_italic(f"Read {len(resp[1:])}/{len(result)}")
        # print_italic(f"Got: {repr(resp)}") 
        # time.sleep(0.1)

    return result


def send(dev, checksum):
    data = b"\x00" # Dummy descriptor header
    data += TYPE_SEND
    data += checksum.to_bytes()
    dev.write(data)
    if (resp := wait_reply(dev)) != RESP_OK:
        raise Exception( f"Send error, resp: {hexlify(resp).decode()}" )
    print_italic("Buffered content sent")

def stop(dev):
    data = b"\x00" # Dummy descriptor header
    data += TYPE_STOP
    dev.write(data)
    if (resp := wait_reply(dev)) != RESP_OK:
        raise Exception( f"Stop error, resp: {hexlify(resp).decode()}" )
    print_italic("Stopped forwarder")

def wait_reply(dev):
    resp = b''
    while not resp.startswith(DESC_GENERIC):
        resp = dev.read(MAX_REPORT_SIZE)
    return resp[len(DESC_GENERIC):]

def flash(dev, path="build/board_B_app_img.bin", offset=IMG_OFFSET):
    total_size = os.path.getsize(path)
    addr = offset
    img = b''
    assert(addr % BLOCK == 0)
    with open(path, "rb") as f:
        print_italic(f"Sync with bootloader")

        payload = b'SYNC'
        ck = write(dev, payload)
        send(dev,ck)
        resp = display_resp(dev)
        assert(resp[:4] == b'PICO')

        with ProgressBar() as pb:
            for _ in pb(iter(int, 1), total=(total_size//SECTOR + 1)):
                if addr % BLOCK == 0:
                    print_italic(f"Erasing block at 0x{addr:08X}")

                    payload = b'ERAS'
                    payload += addr.to_bytes(4, "little", signed=False)
                    payload += BLOCK.to_bytes(4, "little", signed=False)
                    ck = write(dev, payload)
                    send(dev, ck)
                    resp = display_resp(dev)
                    assert(resp[:4] == b'OKOK')

                sector = f.read(SECTOR)
                if not sector:
                    break
                img += sector
                if len(sector) < SECTOR:
                    pad = SECTOR - len(sector)
                    print_italic(f"Padding {pad} at 0x{addr:08X}")
                    sector += b'\x00'* pad
                
                print_italic(f"Writing sector at 0x{addr:08X}")
                sector_crc = crc32(sector)
                print_bold(f"Data CRC: 0x{sector_crc:08X}")

                payload = b'WRIT'
                payload += addr.to_bytes(4, "little", signed=False)
                payload += SECTOR.to_bytes(4, "little", signed=False)
                payload += sector
                ck = write(dev, payload)
                send(dev, ck)
                resp = display_resp(dev)
                assert(resp[:4] == b'OKOK')
                returned_crc = int.from_bytes(resp[4:8], "little", signed=False)
                assert(returned_crc == sector_crc)
                print_italic("CRC Matched")

                addr += SECTOR
    
    print_italic("Sealing image")

    payload = b'SEAL'
    payload += offset.to_bytes(4, "little", signed=False)
    payload += len(img).to_bytes(4, "little", signed=False)
    payload += crc32(img).to_bytes(4, "little", signed=False)
    ck = write(dev, payload)
    send(dev, ck)
    resp = display_resp(dev)
    assert(resp[:4] == b'OKOK')

def flashtest(dev, path="build/board_B_app_img.bin"):

    sector = SECTOR
    num_sector = BLOCK / SECTOR

    print_bold(f"Try write four sectors (one block) of flash from {path}")

    print_bold(f"Erasing first")
    payload = b'ERAS'
    payload += IMG_OFFSET.to_bytes(4, "little", signed=False)
    payload += (sector*num_sector).to_bytes(4, "little", signed=False)
    ck = write(dev, payload)
    send(dev, ck)
    display_resp(dev)

    with open(path, "rb") as f:
        for i in range(num_sector):
            print_bold(f"Writing {i+1}/{num_sector}")
            data = f.read(sector)
            print_bold(f"Data CRC: 0x{crc32(data):08X}")
            payload = b'WRIT'
            payload += (IMG_OFFSET+ i*sector).to_bytes(4, "little", signed=False)
            payload += sector.to_bytes(4, "little", signed=False)
            payload += data
            ck = write(dev, payload)
            send(dev, ck)
            display_resp(dev)

def handle_meta(op, args, dev):
    if op == "STOP":
        stop(dev)
        print_bold("Forwarder stopped")
        return True
    if op == "EXIT":
        exit(0)
    if op == "FLASHTEST":
        flashtest(dev, *args)
        return True
    if op == "FLASH":
        flash(dev, *args)
        return True
    if op == "LE":
        parse_le(args[0])
        return True

def parse_le(le):
    assert(len(le) % 2 == 0)
    be = ''
    for i in reversed(range(len(le)//2)):
        be += le[2*i: 2*i+2]
    print_bold(f"0x{be}")
    print_bold(f"Decimal: {int(be, 16)}")
    

def display_resp(dev):
    resp = read(dev)
    opcode = resp[:4]
    print_bold(f"Response: {opcode.decode()}")
    if resp[4:]:
        print_bold(hexlify(resp[4:]).decode())
    return resp

def main():
    prp = PromptSession('Board_B bootloader>')
    dev = hid.Device(VID, PID)
    while True:
        try:
            cmd = prp.prompt()
            cmd = cmd.split()
            op = cmd[0].upper()
            args = cmd[1:]
            if handle_meta(op, args, dev):
                continue
            op = op.encode()
            assert len(op) == 4, "OPCODE must be length of 4"
            args = [int(i, 16).to_bytes(4, "little", signed=False) for i in args]
            data = op + b''.join(args)
            ck = write(dev, data)
            send(dev, ck)
            if op == b'BOOT':
                # We are not expecting a reply after BOOT
                continue
            display_resp(dev)
        except EOFError:
            break
        except Exception as e:
            print_red(f"Exception: {repr(e)}")
            print_red(f"Full trace:")
            traceback.print_exc()

if __name__ == "__main__":
    main()