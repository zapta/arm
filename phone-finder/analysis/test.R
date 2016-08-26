#!/usr/local/bin/Rscript

f1 = 700
f2 = 1600

# Sampling frequency HZ
sf <- 100000
paste("Sampling freq :", sf, "[HZ]")

# Total sampling period secs
sp <- 0.01
paste("Total sampling period :", sp, "[sec]")

# Sampling interval secs
si <- 1/ sf
paste("Sampling interval :", si, "[sec]")

# Generate sampling time series in sec
ts <- seq(0, sp, si)

wave.1 <- sin(f1*2*pi*ts)
wave.2 <- sin(f2*2*pi*ts)

X11(width=10, height=8)

par(mfrow = c(3, 1))

plot(ts, wave.1, type="l", ylim=c(-1,1)) 
abline(h=0,lty=3)

plot(ts,wave.2, type="l", ylim=c(-1,1)) 
abline(h=0,lty=3)

message("Press return to exit...")
invisible(readLines("stdin", n=1))




