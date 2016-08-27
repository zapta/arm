#!/usr/local/bin/Rscript

options(warn = 1)

f1 = 700
f2 = 1600

paste("f1: ", f1)
paste("f2: ", f2)

# Sampling frequency HZ
sf <- 20000
paste("Sampling freq :", sf, "[HZ]")

# Total sampling period secs
sp <- 0.1 
paste("Total sampling period :", sp, "[sec]")

# Sampling interval secs
si <- 1/ sf
paste("Sampling interval :", si, "[sec]")

# Generate sampling time series in sec
ts <- seq(0, sp, si)

n = length(ts)
paste("n: ", n)

#wave.1 <- sin(f1*2*pi*ts)
#wave.2 <- sin(f2*2*pi*ts)

# Return phase in the range [0, 2pi).
# t is time in secs. f is requency in hz.
time_to_phase <- function(t, f) {
  cycles = t * f
  phase = cycles * 2 * pi
  return (phase %% (2*pi))
}

square_wave <- function(phase) {
  if (phase < pi) {
    return (0)
  }
  return (1)
}

tri_level_wave <- function(phase) {
  k = 0.25
  if (phase < pi*(1-k)) {
    return (1)
  }
  if (phase < pi) {
    return (0)
  }
  if (phase < pi*(2-k)) {
    return (-1)
  }
  return (0)
}

function_f1 <- function(t){
  phase = time_to_phase(t, f1)
  #return (sin(phase))
  #return (square_wave(phase))
  return (tri_level_wave(phase))
}
wave.1 <- as.numeric(lapply(ts, function_f1) )

function_f2 <- function(t){
  phase = time_to_phase(t, f2)
  #return (square_wave(phase))
  return (tri_level_wave(phase))
}
wave.2 <- as.numeric(lapply(ts, function_f2) )

signal <- (wave.1 + wave.2)

y = fft(signal)

mag <- sqrt(Re(y)^2 + Im(y)^2)*2/n

# See here voltage to dbu/dbv conversion
dbu <- 20 * log10(mag/1)

# frequency points hz.
f <- (1:n)/sp

X11(width=10, height=8)

par(mfrow = c(3, 1))

# For the voltage display, we zoom in to 5ms to getter better resultion
n1 <- 0.005/si

plot(ts[1:n1], wave.1[1:n1], type="l") 
abline(h=0,lty=3)

plot(ts[1:n1], wave.2[1:n1], type="l") 
abline(h=0,lty=3)

plot(ts[1:n1], signal[1:n1], type="l") 
abline(h=0,lty=3)

# We want to display up to 5Hkz and in not case more than half
# of the fft result (mirrored on both halves).
fdisplay = 5000  # hz
dn = min((fdisplay/(1/sp)), n/2)

paste("Display points: ", dn)

X11(width=10, height=8)

# Skipping first frequency band (close to DC)
plot(f[2:dn], dbu[2:dn], type="l", ylim=c(-60,0) ) 
abline(h=c(-10, -20, -30, -40), lty=3)

message("Press return to exit...")
invisible(readLines("stdin", n=1))




