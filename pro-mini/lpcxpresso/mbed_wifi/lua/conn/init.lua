--uart.setup(0, 115200, 8, 0, 1, 1)
tmr.delay(100000)
print("\n\ninit.lua")
function RUN() 
  node.compile("config.lua")
  node.compile("conn.lua")
  dofile("config.lc")
  dofile("conn.lc")
end
-- Starting without the timer fails due to insufficient memory.
tmr.alarm(1, 1000, 0, function() 
  RUN()
end)
