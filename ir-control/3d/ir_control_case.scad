// Open Scad 3D printed model of a case for the IR Controller.

$fn=64;

eps1 = 0.01;
eps2 = eps1 + eps1;

// Radius of PCB corners. The value here should be <= the actual
// PCB radius. The radiuses of the base and cover are derived from
// this value.
pcb_corner_radius = 2.8;

// PCB length. (on x axis)
pcb_length = 40;

// PCB width. (on y axis)
pcb_width = 45;

// PCB thickness, without the components.
pcb_thickness = 1.6;

// For tallest component.
pcb_to_cover_clearance = 9;

// Thickness of tape used to stick the PCB to the base.
// Scotch 414P Extreme Mounting Tape.
sticky_tape_thickness = 1.0;

// Base thickness at the center.
base_height = 7;

// Base thickness around the edge.
base_step_height = 2;

// Base margin aroung the PCB. This compensates for PCB
// tolerances and allow the cover's snag-fit to clear the PCB.
// This dimension is on all four sides of the PCB.
pcb_to_base_margin = 1.0;

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

echo("cover_length", cover_length);

// External width of the cover.
cover_width = base_width + 2*base_to_cover_margin + 2*cover_thickness;

// The total height of the cover. This doesn't include the height of 
// the step around the base.
cover_height = (base_height - base_step_height) + sticky_tape_thickness + pcb_thickness  + pcb_to_cover_clearance + cover_thickness; 

// The external radius of the cover's corners.
cover_corner_radius = base_corner_radius + base_to_cover_margin + cover_thickness;

// Distance between centers of base screw holes.
base_screws_spacing = 30;

// Adding compensation for first layer width.
led_hole_length = 5 + 0.6;
led_hole_width = 2 + 0.6;

release_notch_width = 8;
release_notch_height = 1;

pcb_surface_height = base_height + sticky_tape_thickness + pcb_thickness; 
echo("pcb_surface_height", pcb_surface_height);

snap_fit_height = 2;
snap_fit_length = 30;
snap_fit_depth = 0.7;

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

// A hole in the cover for a connector on the USB side.
// eagle_y: center y coordinate in eagle.
// vert_offset: offset of hole vertical center from PCB surface.
module usb_conn_hole(width, height, eagle_y, vert_offset) {
    horiz_offset = eagle_y - pcb_width/2;
    translate([-cover_length/2 - cover_thickness, 
               -width/2 + horiz_offset, 
               pcb_surface_height - base_step_height - height/2 + vert_offset]) 
        cube([3*cover_thickness, 
              width, 
              height]);
}

module usb_conn_holes() {
  width = 12.5;
  height = 8;
  vert_offset = 1.2; //-2.8;
  
  usb_conn_hole(width, height, 37, vert_offset);
  usb_conn_hole(width, height, 20, vert_offset);
  usb_conn_hole(width, height,  8, vert_offset);
}

module ir_conn_hole() {
  eagle_y = 35;
  vert_offset = 2.5;
  diameter = 8;
 
  horiz_offset = eagle_y - pcb_width/2;
  echo("horiz offset", horiz_offset);
  
  translate([cover_length/2 - cover_thickness/2, 
               horiz_offset, 
               pcb_surface_height - base_step_height  + vert_offset]) 
        rotate([0, 90, 0]) 
           cylinder(d=diameter, h=3*cover_thickness, center=true);       
}

module led_hole() {
  // Slopt, for easy LED insertion
  expansion = 0.7;
  // Since we flash the board edge with connector on the base edge, the entire board
  // including the LED is slighly offseted.
  //correction_offset = -0.8;
  // '15' is the nominal led center distance from the PCB center.
  translate([13, -10.5, cover_height - cover_thickness])
  union() {
    hull() {
      cube([led_hole_width+2*expansion, led_hole_length+2*expansion, eps2], center=true);  
      translate([0, 0, cover_thickness*2/3]) 
        cube([led_hole_width, led_hole_length, eps2], center=true);  
    }
    cube([led_hole_width, led_hole_length, 2*(cover_thickness+eps1)], center=true); 
  }
}

module base_snap_fit_holes() {
  module sf(a) {
    dz = base_step_height + (base_height - base_step_height)/2;
    dy =  base_width/2+eps1;   
    rotate([0, 0, a]) translate([0, dy, dz])
      snap_fit(snap_fit_height, snap_fit_depth, snap_fit_length+4);
  }
  
  sf(0);
  sf(180);
}

module cover_snap_fit_bumps() {
  dz = (base_height - base_step_height)/2;
  dy = cover_width/2 - cover_thickness + eps1;
  
  translate([0, dy, dz]) snap_fit(snap_fit_height, snap_fit_depth, snap_fit_length);
  
  mirror([0, 1, 0]) translate([0, dy, dz]) snap_fit(snap_fit_height, snap_fit_depth, snap_fit_length);
}

module release_notches() {
  translate([-release_notch_width/2, -cover_width/2-eps1, -eps1])
  cube([release_notch_width, cover_width + eps2, release_notch_height+eps1]);
}

// Engraved text on the top of the cover.
//module cover_text(x, y, angle, msg) {
//  depth = 1;
//  size = 3;
//   translate([x, y, cover_height-depth]) linear_extrude(height = depth+eps1) rotate([0, 0, angle]) text(msg, halign="center",valign="center", size=size, font="Helvetica:style=Bold");
//}

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
    usb_conn_holes();
    ir_conn_hole();
    //phone_conn_hole();
    led_hole();
    release_notches();
    
    // text
    // Tweak the dimension to match the holes.
//    cover_text(-18.5, -14, -90, "TV");
//    cover_text(-18.5, -2, -90, "AV");
//    cover_text(-18.5, 14.6, -90, "PWR");  
//    cover_text(18.5, 10.7, 90, "IR");

  }
  cover_snap_fit_bumps();
}

// Cavities in the base for PCB features. 
// Input coordiantes are in eagle x,y.
module pcb_sink(eagle_x, eagle_y, d) { 
  x =  -pcb_length/2 + eagle_x;
  y =  - pcb_width/2 + eagle_y;
  depth = 2;
  translate([x, y, base_height - depth]) cylinder(d=d, h=depth+eps1);
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
    
    base_snap_fit_holes();
    
    // Clearance for phone jack mount tabs.
    pcb_sink(30, 30, 5);
    pcb_sink(30, 40, 5);  
    pcb_sink(35, 35, 5);
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
  color([0, 0, 0.6, 0.5]) 
      translate([0, 0, base_step_height + eps1]) cover();
}

module parts_for_printing() {
  space = 8;
  base();
  translate([cover_length+space, 0, cover_height]) 
       rotate([180, 0, 0])  cover(); 
}

//base();

//cover();



//parts_assembled();

parts_for_printing();

//intersection() {
//   cover();
//   translate([-85, -35, -eps1]) cube([70, 70, 40]); 
//}

//intersection() {
//  parts_for_printing();
//  translate([43, -7, -eps1]) cube([30, 30, 50]);
//}

//difference() {
//  parts_assembled();
//  translate([0, -50, -eps1]) cube([100, 100, 100]);
//}

