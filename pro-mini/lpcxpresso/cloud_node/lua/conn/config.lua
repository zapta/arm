print("config")

wifi.setmode(wifi.STATION)
wifi.sta.autoconnect(0)
  
file.open("_config", "r")

local ssid = file.readline():gsub("%c", "")
print("ssid: ["..ssid.."]")

local password = file.readline():gsub("%c", "")
print("password: ["..password.."]")

gw_host = file.readline():gsub("%c", "")
print("gw_host: ["..gw_host.."]")

local gw_port_str = file.readline():gsub("%c", "")
gw_port = tonumber(gw_port_str)
print("gw_port: ["..gw_port.."]")

file.close()
wifi.sta.config(ssid, password)

