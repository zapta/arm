
$fn=64;

eps1 = 0.01;
eps2 = eps1 + eps1;

board_corner_radius = 3;
board_mount_length = 43;
board_mount_witdh = 43;
board_length = 50;
board_width = 50;
board_thickness = 1.6;

base_thickness = 3;
base_post_height = 4;  // above base.
board_base_margin = 0.2;

base_corner_radius = board_corner_radius + board_base_margin;


base_length = board_length + 2*board_base_margin;
base_width = board_width + 2*board_base_margin;


cover_thickness = 2;
base_to_cover_margin = 0.2;
board_to_cover_clearance = 13;

cover_length = base_length + 2*base_to_cover_margin + 2*cover_thickness;
cover_width = base_width + 2*base_to_cover_margin + 2*cover_thickness;
cover_height = base_thickness + board_thickness + board_to_cover_clearance + cover_thickness; 
cover_corner_radius = base_corner_radius + base_to_cover_margin + cover_thickness;


// Hole for a M3 metal insert, mcmaster part number 94180A331.
// h is the total depth for the screw hole. Already includes an
// extra eps1 at the opening side.
module m3_threaded_insert(h) {
  // Adding tweaks for compensate for my printer's tolerace.
  A = 5.1 + 0.3;
  B = 5.31 + 0.4;
  L = 3.8;
  D = 4.0;
  translate([0, 0, -eps1]) {
    // NOTE: diameter are compensated to actual print results.
    // May vary among printers.
    cylinder(d1=B, d2=A, h=eps1+L*2/3, $f2=32);
    cylinder(d=A, h=eps1+L, $f2=32);
    translate([0, 0, L-eps1]) cylinder(d=D, h=h+eps1-L, $f2=32);
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

// A single base post without the insert hole. 
module uncliped_base_post_body() {
  h = base_thickness + base_post_height;
  d = 8;
  stretch = d / 2;
  hull() {
    cylinder(d=d, h=h);
    translate([-d/2, -d/2-stretch, 0]) cube([d, eps1, h]);
    translate([-d/2-stretch, -d/2, 0]) cube([eps1, d, h]);
    //rotate([0, 0, -45]) translate([0, -d/2-stretch/2, 0]) 
    //    cylinder(d=stretch, h=h);
 }
}

// Hole for threaded insert. Already elevated to match uncliped base post body.
module base_post_hole() {
  h = base_thickness + base_post_height;
  translate([0, 0, h+eps1]) mirror([0, 0, 1]) m3_threaded_insert(5);  
}

module cover() {
  
  difference() {
   rounded_box(cover_length, cover_width, cover_height, cover_corner_radius);
   translate([0, 0, -eps1]) 
     rounded_box(
         cover_length-2*cover_thickness, 
         cover_width-2*cover_thickness, 
         cover_height-cover_thickness+eps1, 
         cover_corner_radius - cover_thickness, 
         0, 4);
  }
}

module base_plate(h) {
  rounded_box(base_length, base_width, h, base_corner_radius);
}

module base() {
  dx = board_mount_length/2;
  dy = board_mount_length/2;
  difference() {
    // Combine base and posts.
    union() {
      base_plate(base_thickness);
      intersection() {
        translate([0, 0, -eps1]) base_plate(10);
        union() {
          translate([-dx, -dy, 0]) uncliped_base_post_body();
          translate([-dx, dy, 0]) mirror([0, 1, 0]) uncliped_base_post_body();
          translate([dx, -dy, 0]) mirror([1, 0, 0]) uncliped_base_post_body();
          translate([dx, dy, 0]) mirror([1, 1, 0]) uncliped_base_post_body();
        }
      } 
    }
    
    // Make holes for inserts.
    translate([-dx, -dy, 0]) base_post_hole();
    translate([-dx, dy, 0]) base_post_hole();
    translate([dx, -dy, 0]) base_post_hole();
    translate([dx, dy, 0]) base_post_hole();
  }
}


module board_hole() {
  translate([0, 0, -eps1]) cylinder(d=3.2, h=board_thickness+eps2);
}

module board() {
  dx = board_mount_length/2;
  dy = board_mount_length/2;
  color([0, 0.6, 0, 0.6]) difference() {
    rounded_box(board_length, board_width, board_thickness, board_corner_radius);
    translate([-dx, -dy, 0]) board_hole();
    translate([-dx, dy, 0]) board_hole();
    translate([dx, -dy, 0]) board_hole();
    translate([dx, dy, 0]) board_hole();
  }
}


//intersection() {
  //translate([0, 0, -eps1]) rotate([0, 0, 45]) cube([100, 100, 100]);
  union() {
  translate([0, 0, eps1]) cover();
  base();
  translate([0, 0, base_thickness + base_post_height + eps1]) board();
  }
//}
