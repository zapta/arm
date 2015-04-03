#!/usr/bin/python

import getopt
import optparse
import os
import serial
import sys
import string
import time

# Globals. Set later.
FLAGS = None
ser = None

def debug(str):
  if FLAGS.debug:
    print(str)

# Parse args and set FLAGS. Return a list of 
# positional (unnamed) args.
def parseArgs(argv):
  global FLAGS
  parser = optparse.OptionParser()
  parser.add_option(
    "-p", "--port", dest="port",
    #default='/dev/tty.usbserial-A602VGAX',
    default='/dev/tty.usbmodemfd111',
    help="serial port connected to ESP8266")
  parser.add_option(
    "-s", "--speed", dest="speed",
    #default="115200",
    default="9600",
    help="ESP8266 serial speed")
  parser.add_option(
    "-d", "--debug", dest="debug",
    default=False,
    help="show verbose debug information")
  (FLAGS, args) = parser.parse_args()
  debug('Flags:')
  debug('  --port  = [%s]' % FLAGS.port)
  debug('  --speed = [%s]' % FLAGS.speed)
  debug('  --debug = [%s]' % FLAGS.debug)
  debug('Command:')
  debug('  %s' % args[:1])
  debug('Args:')
  debug('  %s' % args[1:])
  return args

def esc_char_for_display(c):
  if c == '\r':
    return '{\\r}'
  if c == '\n':
    return '{\\n}'
  if (c >= ' ' and c <= '~'):
    return c
  return '{\\%03o}' % ord(c)

def esc_str_for_display(str):
  result = ''
  for c in str:
    result += esc_char_for_display(c)
  return result

def esc_data_char_for_tx(c):
  if (c >= ' ' and c <= '~' and c != '\\' and c != '"'):
    return c
  # NOTE: Lua uses DECIMAL escape values.
  return '\\%03d' % ord(c)

def tx(str):
  debug('TX [%s]' % esc_str_for_display(str))
  ser.write(str)

def rx_till_prompt():
  data = ''
  while data[-2:] != '> ':
    data += ser.read()
    # Print partial RX.
    debug('RX [%s]' % esc_str_for_display(data))
  debug('RX [%s] (done)' % esc_str_for_display(data))
  # Remove the prompt and trip leading and trailing white space.
  return data[:-2].strip()

def esp8266_cmd(str):
  time.sleep(0.1)
  tx(str+'\n')
  return rx_till_prompt();

def setup_no_echo():
  esp8266_cmd('uart.setup(0, %s, 8, 0, 1, 0)' % FLAGS.speed)

def cmd_reset(argv):
  print("Reseting ESP8266 board...")
  esp8266_cmd('node.restart()')
  while True:
    resp = rx_till_prompt()
    print('RESP: [%s]' % resp)
    if "NodeMCU" in resp:
      break
  setup_no_echo();
  print("Reset done.")

# Syntax: push_files <local_file_name>...
def cmd_push_files(args):
  print("cmd_push_files")
  for local_file_name in args:
    print("\n--- %s" % local_file_name)
    # Read source file.
    f = open(local_file_name, 'rb' )
    content = f.read()
    f.close()
    print
    print('File size: [%d] bytes' % len(content))
  
    # Compute destination file name.
    remote_file_name = os.path.basename(local_file_name)
  
    # Open remote file for writing.
    print
    print("Writing ESP8266 file [%s]:" % remote_file_name)
    esp8266_cmd('file.remove("%s") file.open("%s", "w") w = file.write f = file.flush' % (remote_file_name, remote_file_name))
  
    # Send file data.
    max_bytes_per_line = 100
    data = ''
    for i, char in enumerate(content):
      data += esc_data_char_for_tx(char)
      if len(data) >= max_bytes_per_line or i == len(content) - 1:
        esp8266_cmd('w("%s") f()' % data)
        data = ''
        print str(int(100 * i / (len(content) - 1))).rjust(3), '%'
    
    esp8266_cmd('file.close()')

#@@@@
# Syntax: get_file <remote_file_name>.
def cmd_pull_file(args):
  file_name = args[0]

  # Read source file.
  #f = open(local_file_name, 'rb' )
  #content = f.read()
  #f.close()
  #print
  #print('File size: [%d] bytes' % len(content))

  # Compute destination file name.
  #remote_file_name = os.path.basename(local_file_name)

  # Open remote file for writing.
  print
  print("Reading ESP8266 file [%s]:" % file_name)
  esp8266_cmd('file.open("%s", "r")' % (file_name))
  esp8266_cmd('repeat b=file.read(40) if b then print("DATA["..b.."]> ") end until not b print("END> ")')

  while True:
    resp = rx_till_prompt()
    print('RESP: [%s]' % resp)
    #if resp == "CONNECTED":
    #  break

  # Read data
  #max_bytes_per_line = 100
  #data = ''

  #for i, char in enumerate(content):
  #  data += esc_data_char_for_tx(char)
  #  if len(data) >= max_bytes_per_line or i == len(content) - 1:
  #    esp8266_cmd('w("%s") f()' % data)
  #    data = ''
  #    print str(int(100 * i / (len(content) - 1))).rjust(3), '%'
  
  esp8266_cmd('file.close()')

# Delete one or more files from the ESP8266 file storage.
# Syntax: delete_files <remote_file_name>...
def cmd_delete_files(args):
  for remote_file_name in args:
    print("Deleting ESP8266 file [%s]:" % remote_file_name)
    esp8266_cmd('file.remove("%s")' % remote_file_name)


# List the files on the ESP8266 file storage.
# Syntax: list_files
def cmd_list_files(args):
  esp8266_cmd('l = file.list();')
  response = esp8266_cmd('print("File list:") for k,v in pairs(l) do print("  "..k..": "..v.." bytes")  end')
  print(response)
 

# Run a test script. For experimentation..
# Syntax: test <ap_ssid> <ap_password>
def cmd_test_script(args):
  ssid = args[0]
  password = args[1]


  print("Connecting to AP (%s, %s)..." % (ssid, password))
  esp8266_cmd('print(wifi.setmode(wifi.STATION))')
  esp8266_cmd('wifi.sta.config("%s","%s")' % (ssid, password))
  esp8266_cmd('wifi.sta.connect()' % ())
  # Wait for AP connection.
  while True:
    resp = esp8266_cmd('print(wifi.sta.status())')  # expecting '5' for ok
    print('Status: [%s]' % resp)
    if resp == '5':
      print('Connected to AP')
      break;
    time.sleep(1)

  print('Connecting to server...')
  #esp8266_cmd('conn = net.createConnection(net.TCP, 1)')
  esp8266_cmd('conn = net.createConnection(net.TCP, 0)')

  #esp8266_cmd('conn:on("receive", function(conn, str) print(str) end)')
  esp8266_cmd('conn:on("receive", function(conn, str) print(string.format("RECEIVE %04d> ", string.len(str))) end)')

  esp8266_cmd('conn:on("connection", function(conn) print("CONNECTED> ") end)')
  esp8266_cmd('conn:on("reconnection", function(conn) print("RECONNECTED> ") end)')
  esp8266_cmd('conn:on("disconnection", function(conn) print("DISCONNECTED> ") end)')
  esp8266_cmd('conn:on("sent", function(conn) print("SENT> ") end)')

  #esp8266_cmd('conn:connect(443, "www.google.com")')
  esp8266_cmd('conn:connect(9000, "192.168.0.90")')

  while True:
    resp = rx_till_prompt()
    print('RESP: [%s]' % resp)
    if resp == "CONNECTED":
      break

  print('Sending query...')
  #esp8266_cmd('conn:send("GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\nConnection: keep-alive\\r\\nAccept: */*\\r\\n\\r\\n")')
  esp8266_cmd('conn:send("GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\nConnection: close\\r\\nAccept: */*\\r\\n\\r\\n")')

  while True:
    resp = rx_till_prompt()
    print('RESP: [%s]' % resp)
    if resp == "DISCONNECTED":
      break

  print('test_script done.')


def main(argv):
  args = parseArgs(argv)
  print("%s@%s" % (FLAGS.port, FLAGS.speed))

  cmd_table = {
    'push_files' : cmd_push_files,
    'push' : cmd_push_files,

    'list_files' : cmd_list_files,
    'list_file' : cmd_list_files,
    'list' : cmd_list_files,
    'ls' : cmd_list_files,

    'delete_files' : cmd_delete_files,
    'delete' : cmd_delete_files,
    'rm' : cmd_delete_files,

    'reset' : cmd_reset,

    'pull_file' : cmd_pull_file,
    'pull' : cmd_pull_file,

    'test_script' : cmd_test_script,
  }

  cmd_name = args[0]
  cmd_args = args[1:]

  if cmd_table.has_key(cmd_name):
    # Open serial port to ESP8266 and disable ESP8266 echo.
    global ser
    ser = serial.Serial(FLAGS.port, FLAGS.speed, timeout=1)
    setup_no_echo();
    #esp8266_cmd('uart.setup(0, %d, 8, 0, 1, 0)' % FLAGS.speed)

    # Run the command
    cmd_handler = cmd_table[cmd_name]
    cmd_handler(cmd_args)

    # Close
    ser.close()
    return

  print('Unknown command: [%s]' % cmd_name)
  print('Aborting')
  sys.exit(1)

if __name__ == "__main__":
  main(sys.argv[1:])
