#!/usr/local/bin/Rscript

f1 = 700
f2 = 1600

# Sampling frequency HZ
sf <- 100000
paste("Sampling freq :", sf, "[HZ]")

# Total sampling period secs
sp <- 1 
paste("Total sampling period :", sp, "[sec]")

# Sampling interval secs
si <- 1/ sf
paste("Sampling interval :", si, "[sec]")

# Generate sampling time series in sec
ts <- seq(0, sp, si)

n = length(ts)
paste("n: ", n)

wave.1 <- sin(f1*2*pi*ts)
wave.2 <- sin(f2*2*pi*ts)

wave.3 <- (wave.1 + wave.2)

y = fft(wave.3)

mag <- sqrt(Re(y)^2 + Im(y)^2)*2/n

# See here voltage to dbu/dbv conversion
dbu <- 20 * log10(mag/1)

# frequency points hz.
f <- (1:n)/sp

X11(width=10, height=8)

par(mfrow = c(3, 1))

# For the voltage display, we zoom in to 5ms to getter better resultion
n1 <- 0.005/si

plot(ts[1:n1], wave.1[1:n1], type="l", ylim=c(-1,1)) 
abline(h=0,lty=3)

plot(ts[1:n1], wave.2[1:n1], type="l", ylim=c(-1,1)) 
abline(h=0,lty=3)

# We want to display up to 5Hkz and in not case more than half
# of the fft result (mirrored on both halves).
fdisplay = 5000  # hz
dn = min((fdisplay/(1/sp)), n/2)

paste("Display points: ", dn)


plot(f[1:dn], dbu[1:dn], type="l", ylim=c(-75,0) ) 
abline(h=0,lty=3)

message("Press return to exit...")
invisible(readLines("stdin", n=1))




