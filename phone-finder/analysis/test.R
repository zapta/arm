#!/usr/local/bin/Rscript


xs <- seq(-2*pi,2*pi,pi/100)

wave.1 <- sin(3*xs)
wave.2 <- sin(10*xs)


X11(width=10, height=8)

par(mfrow = c(3, 1))

plot(xs,wave.1, type="l", ylim=c(-1,1)) 
abline(h=0,lty=3)

plot(xs,wave.2, type="l", ylim=c(-1,1)) 
abline(h=0,lty=3)

message("Press return to exit...")
invisible(readLines("stdin", n=1))




