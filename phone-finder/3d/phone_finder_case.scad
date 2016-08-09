// Open Scad 3D printed model of a case for the Phone Finder.

$fn=64;

eps1 = 0.01;
eps2 = eps1 + eps1;

// Radius of PCB corners. The value here should be <= the actual
// PCB radius. The radiuses of the base and cover are derived from
// this value.
pcb_corner_radius = 2.8;

// PCB length
pcb_length = 50;

// PCB width.
pcb_width = 50;

// PCB thickness, without the components.
pcb_thickness = 1.6;

// For tallest component.
pcb_to_cover_clearance = 13;

// Thickness of tape used to stick the PCB to the base.
// Scotch 414P Extreme Mounting Tape.
sticky_tape_thickness = 1.0;

// Base thickness at the center.
base_height = 7;

// Base thickness around the edge.
base_step_height = 2;

// Base margin aroung the PCB.
pcb_to_base_margin = 0.4;

// Corner radius of the thick part of the base.
base_corner_radius = pcb_corner_radius + pcb_to_base_margin;

// The length of the thick part of the base, without the side step.
// This is the length of the area that supports the PCB.
base_length = pcb_length + 2*pcb_to_base_margin;

// The widthof the thick part of the base, without the side step.
// This is the width of the area that supports the PCB.
base_width = pcb_width + 2*pcb_to_base_margin;

// Cover top and side wall thickness.
cover_thickness = 2;

// Ajust for tight cover fit.
base_to_cover_margin = 0.1;

// External length of the cover.
cover_length = base_length + 2*base_to_cover_margin + 2*cover_thickness;

// External width of the cover.
cover_width = base_width + 2*base_to_cover_margin + 2*cover_thickness;

// The total height of the cover. This doesn't include the height of 
// the step around the base.
cover_height = (base_height - base_step_height) + sticky_tape_thickness + pcb_thickness  + pcb_to_cover_clearance + cover_thickness; 

// The external radius of the cover's corners.
cover_corner_radius = base_corner_radius + base_to_cover_margin + cover_thickness;

// Distance between centers of base screw holes.
base_screws_spacing = 30;

pcb_surface_height = base_height + sticky_tape_thickness + pcb_thickness; 

module snap_fit(d, h, l) {
  d1 = d;
  d2 = d * 0.2;
  rotate([90, 0, 0])
  hull() {
    translate([(l-d1)/2, 0, 0]) cylinder(d1=d1, d2=d2, h=h); //point_side_bump(d, h);
    translate([-(l-d1)/2, 0, 0]) cylinder(d1=d1, d2=d2, h=h); //point_side_bump(d, h);
  }
}

// A cylinder with rounded bottom.
// r is the corner radius at the bottom.
// Similar to rounded_cylinder(d, h, r, 0).
// If r=0 then similar to cylinder(d, h).
module rounded_cylinder1(d, h, r) {
  if (r > 0) {
    intersection() {
      translate([0, 0, r])
      minkowski() {
        cylinder(d=d-2*r, h=h*2-2*r);
        sphere(r=r);  
      }
      translate([0, 0, -eps1])
      cylinder(d=d+eps1, h=h+eps1);
    }
  } else {
    cylinder(d=d,h=h);
  }
}

// A cylinder with both bottom and top rounded.
// r1 (r2) is the cornder radius at the bottom (top).
// If r2 = 0 then similar to rounded_cylinder1().
// If r1=r2=0 then similar to cylinder (d, h).
module rounded_cylinder(d, h, r1, r2) {
  // We make it from two half with rediuses r1, r2 respectivly.
  h1 = r1 + (h - r1- r2)  / 2;
  h2 = h - h1;
  
  // Bottom half
  rounded_cylinder1(d, h1+eps1, r1);
  
  // Top half
  translate([0, 0, h])
  mirror([0, 0, 1]) 
  rounded_cylinder1(d, h2, r2);
}

// A  box with rounded side, bottom and top corners.
// r is the side corner radius. 
// r1 (r2) is the cornder radius at the bottom (top).
module rounded_box(l, w, h, r, r1=0, r2=0) {
  dx = l/2 - r;
  dy = w/2 - r;
  hull() {
    translate([-dx, -dy, 0]) rounded_cylinder(2*r, h, r1, r2); 
    translate([-dx, dy, 0]) rounded_cylinder(2*r, h, r1, r2); 
    translate([dx, -dy, 0]) rounded_cylinder(2*r, h, r1, r2); 
    translate([dx, dy, 0]) rounded_cylinder(2*r, h, r1, r2); 
  }
}

// A hole in the cover for a connector.
// horiz_offset: offset from PCB center line.
// vert_offset: offset from PCB surface.
module conn_hole(width, height, horiz_offset, vert_offset) {
    translate([-cover_length/2 - cover_thickness, 
               -width/2 + horiz_offset, 
               pcb_surface_height -base_step_height + vert_offset]) 
        cube([3*cover_thickness, 
              width, 
              height + eps1]);
}

module usb_conn_hole() {
  // TODO: set actual hole dimensions
  conn_hole(11, 6, -15, 1.2);
}

module phone_conn_hole() {
  // TODO: set actual hole dimensions
  conn_hole(10, 7, 15, 3);
}

module base_screw_hole() {
  d1 = 4;
  d2 = 10;
  sink_depth = 4;
  translate([0, 0, -eps1]) cylinder(d=d1, h=base_height+eps2);  
  translate([0, 0, base_height-sink_depth]) cylinder(d=d2, h=sink_depth+eps1);  
}

snap_fit_height = 2;
snap_fit_length = 20;
snap_fit_depth = 0.7;

module base_snap_fit_holes() {
  module sf(a) {
    dz = base_step_height + (base_height - base_step_height)/2;
    dy =  base_width/2+eps1;   
    rotate([0, 0, a]) translate([0, dy, dz])
      snap_fit(snap_fit_height, snap_fit_depth, snap_fit_length+4);
  }
  
  sf(0);
  sf(180);
  
  // If the base is symetric just make it so.
  if (base_width == base_length) {
    sf(90);
    sf(270);
  }
}

module cover_snap_fit_bumps() {
  dz = (base_height - base_step_height)/2;
  dy = cover_width/2 - cover_thickness + eps1;
  
  translate([0, dy, dz]) snap_fit(snap_fit_height, snap_fit_depth, snap_fit_length);
  mirror([0, 1, 0]) translate([0, dy, dz]) snap_fit(snap_fit_height, snap_fit_depth, snap_fit_length);
}

module cover() {
  difference() {
    rounded_box(cover_length, cover_width, cover_height,
                cover_corner_radius, 0, 1);
    translate([0, 0, -eps1]) 
     rounded_box(
         cover_length-2*cover_thickness, 
         cover_width-2*cover_thickness, 
         cover_height-cover_thickness+eps1, 
         cover_corner_radius - cover_thickness, 
         0, 4);
    usb_conn_hole();
    phone_conn_hole();
  }
  cover_snap_fit_bumps();
}

// The base part.
module base() {
  extra = base_to_cover_margin + cover_thickness;
  
  difference() {
    union() {
      rounded_box(base_length, base_width, base_height, base_corner_radius);
      rounded_box(base_length + 2*extra, base_width + 2*extra, base_step_height, 
        base_corner_radius + extra, 0.5, 0.2);
    }
    translate([0, -base_screws_spacing/2, 0]) base_screw_hole();
    translate([0, base_screws_spacing/2, 0]) base_screw_hole();
    translate([-base_screws_spacing/2, 0, 0]) base_screw_hole();
    translate([base_screws_spacing/2, 0, 0]) base_screw_hole();
    
    base_snap_fit_holes();
  }
}

// A piece of plastic at the size of the unpopulated PCB. For simulation.
// No need to print this.
module pcb() {
  color([0.6, 0.6, 0.6, 0.9]) 
    rounded_box(pcb_length, pcb_width, pcb_thickness, pcb_corner_radius, 0.2, 0);
}

// Combine the parts in assembled position, with eps spacing.
module parts_assembled() {
  base();
  translate([0, 0, base_height + sticky_tape_thickness]) pcb();
  color([0, 0, 0.6, 0.7]) 
      translate([0, 0, base_step_height + eps1]) cover();
}

module parts_for_printing() {
  space = 8;
  rotate([0, 0, 90]) base();
  translate([cover_length+space, 0, cover_height]) 
      mirror([0, 0, 1]) rotate([0, 0, 90])  cover(); 
}



//parts_assembled();

parts_for_printing();

//difference() {
//  parts_assembled();
//  translate([0, -50, -eps1]) cube([100, 100, 100]);
//}

