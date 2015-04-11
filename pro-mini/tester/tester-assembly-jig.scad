// Spacers for assembling ARM PRO MINI TERSTER.

w = 13;
l = 40;  
h1 = 8;  // Between traget board and tester auxilary board
h2 = 5;  // Between tester auxilary and main boards.

color("red") cube([w, l, h1]);

translate([w + 5, 0, 0]) color("blue") cube([w, l, h2]);
