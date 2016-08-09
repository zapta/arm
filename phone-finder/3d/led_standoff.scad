// LED PCB standoff generator.

// Number of faces per 360 degrees.
$fn=64;

eps1 = 0.01;
eps2 = eps1 + eps1;

// Outer diamter.
outer_diameter = 4.0;

// Total height.
height = 8.0;

// Spacing betwene the centers of the two leads.
leads_spacing = 2.54;

// The diamter of the lead holes.
leads_hole_diameter = 0.9;

// 0 for a closed hole. 1 for an open slot.
use_open_slot = 1;

// Values below 1.0 reduces the footprint of the standoff at the 
// cost of reduces stability.
width_factor = 0.8;

// Number of rows to generate.
rows = 2;

// Number of columns to generate.
columns = 4;

// Spacing of printed rows and columns.
part_spacing = 5;

// A single LED lead hole or slot. Positioned at the 
// positive X direction. 
module lead_hole() {
  module hole(offset) {
     translate([offset, 0, -eps1])
        cylinder(d=leads_hole_diameter, h=height+eps2);   
  }
  
  if (use_open_slot == 0) {
    hole(leads_spacing/2);
  } else {
    hull() {
      hole(leads_spacing/2);
      hole(outer_diameter);
    }
  }
}

// A round standoff, before applying width_factor.
module round_led_standoff() {
  difference() {
    cylinder(d=outer_diameter, h=height);
    lead_hole();
    mirror([1, 0, 0]) lead_hole();
  }
}

// A round standoff with width_factor applied.
module led_standoff() {
  dx = outer_diameter + eps2;
  dy = (width_factor * outer_diameter) + eps2;
  dz = height + eps2;
  intersection() {
    round_led_standoff();
    translate([-dx/2, -dy/2, -eps1])
      cube([dx, dy, dz]); 
  } 
}

// Generate the array
pitch = outer_diameter + part_spacing;

for (i = [1:columns]) {
  for (j = [1:rows]) {
    translate([i*pitch, j*pitch, 0]) led_standoff();
  }
}

