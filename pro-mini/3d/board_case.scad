// See OpenSCAD documentation at 
// http://en.wikibooks.org/wiki/OpenSCAD_User_Manual/The_OpenSCAD_Language
//
// A simple 3D printed case for Arm Pro Mini. Board can be mounted to the
// bottom of the case with 3M Mounting Tape.
//
// All dimensions are in mm.

$fn=36;

// Length (along x axis).
board_len = 38.01;  
// Width (along y axis)
board_width = 20.32;  
// PCB thicknes.
board_thickness = 1.6;  
// Free marging aroudn the pcb.
board_margin = 0.5; 
// Width of box's wall.
wall_width = 1.6;  
// Radius of four box corners.
corner_radius = wall_width; 
// Thickness of the bottom.
bottom_height = 1.5; 
// Free space to leave below the PCB (for double side tape).
space_below_board = 1;
// Free space to leave above the PCB.
space_above_board = 3;
// Width of opening for USB connector.
port_width = 9;
// Offset of USB connector center from board's center.
port_offset_from_center = 1.27;

// Derived dimensions
//
box_len = 2*wall_width + 2*board_margin + board_len;
box_width = 2*wall_width + 2*board_margin + board_width;
box_height = wall_width + space_below_board + space_above_board;

// A small, >0 value for well define relations between objects.
// Does not affect generated box.
eps=0.01;
eps2 = 2*eps;

module solid_box(x, y, z, xlen, ylen, zlen) {
 translate([x, y, z]) 
 cube([xlen, ylen, zlen]); 
}

module rounded_solid_box(x, y, z, xlen, ylen, zlen, r) {
  translate([x+r, y+r, z]) minkowski() {
    cube([xlen-2*r, ylen-2*r, zlen/2]);
    cylinder(r=r, h=zlen/2);
 }
}

module main_part() {
  difference() {
    // Overall box.
    rounded_solid_box(0, 0, 0, 
        box_len, box_width, box_height, corner_radius);
    // Remove inner space.
    solid_box(wall_width, wall_width, bottom_height, 
        box_len-2*wall_width, 
        box_width-2*wall_width, 
        box_height);
    // Remove opening for USB connector.
    solid_box(-eps, 
        box_width/2-port_offset_from_center - port_width/2, 
        bottom_height, wall_width + eps2, port_width, box_height);
  }
}

main_part();