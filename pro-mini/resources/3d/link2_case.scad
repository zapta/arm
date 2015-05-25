                                                                                                                   // Customizeable rounder box with cover.
// See customizer documentation here http://customizer.makerbot.com/docs

// TODO: clean formatting.
// TODO: handle gracefully inconsistnet parameters. Now can result zero material.


/* [Global] */

// All dimensions are in mm.
// 1 for base only, 2 for cover only, 3 for both.
//part_selector = 1;  // [1:Base,2:Cover,3:Both]
// External length of the base and cover.
external_length = 91;
// External width of the base and cover.
external_width = 44;
// Height of the box without the cover.
base_height = 10;
// Thickness of the open box's walls.
wall_thickness = 2.5;
// Thickness of the bottom.
bottom_thickness = 2.5;
// Radius of the external corners.
external_corner_radius = 3;

cut_width = 13;
// From center line.
cut_center_offset = 1;

//// The external (visible) height of the cover.
//cover_external_height = 5;
//// The internal (invisible) height of the cover.
//cover_internal_height = 10;
//// The real thickness of the cover.
//cover_thickness = 2;
//// Clearance between the box walls and the cover.
//cover_internal_margin = 0.5;
//// The thickiness of the internal walls of the cover.
//cover_wall_thickness = 2;
//// Cover printing offset (when printing both cover and cover)
//cover_printing_y_offset = 10;

/* [Hidden] */

$fn = 180;
eps = 0.001;
eps2 = 2*eps;

base_internal_corner_radius = max(eps, external_corner_radius-wall_thickness);
//cover_internal_corner_radius1 = max(eps, base_internal_corner_radius - cover_internal_margin);
//cover_internal_corner_radius2 = max(eps, cover_internal_corner_radius1 - cover_wall_thickness);
//
//// Move cover when printing both.
//cover_y = (part_selector == 3) ? (external_width + cover_printing_y_offset) : 0;

module rounded_solid_box(xlen, ylen, zlen, r) {
  translate([r, r, 0]) minkowski() {
    cube([xlen-2*r, ylen-2*r, zlen/2]);
    cylinder(r=r, h=zlen/2);
 }
}

module cut() {
  y_offest = external_width/2 +cut_center_offset - cut_width/2;
  translate([-eps, y_offest, bottom_thickness])
    cube([wall_thickness+eps2, cut_width, base_height]); 
}

module main() {
  difference() {
    rounded_solid_box(external_length, external_width, base_height, external_corner_radius);

    translate([wall_thickness, wall_thickness, bottom_thickness]) 
      rounded_solid_box(
          external_length-2*wall_thickness, 
          external_width-2*wall_thickness, 
          base_height, 
          base_internal_corner_radius);
    cut();
  }
}

//module cover() {
//  difference() {
//    union() {
//      translate([0, cover_y, 0])
//      rounded_solid_box(external_length, external_width, cover_external_height, external_corner_radius);
//   
//      translate([
//        wall_thickness + cover_internal_margin,
//        cover_y + wall_thickness + cover_internal_margin,
//        cover_external_height
//      ])
//        rounded_solid_box(
//          external_length - 2*(wall_thickness + cover_internal_margin) , 
//          external_width - 2*(wall_thickness + cover_internal_margin), 
//          cover_internal_height + eps, cover_internal_corner_radius1);
//    }
//      translate([
//          wall_thickness + cover_internal_margin + cover_wall_thickness,
//          cover_y + wall_thickness + cover_internal_margin + cover_wall_thickness,
//        cover_thickness - eps])
//        rounded_solid_box(
//            external_length - 2*(wall_thickness + cover_internal_margin + cover_wall_thickness), 
//            external_width - 2*(wall_thickness + cover_internal_margin+cover_wall_thickness), 
//            cover_external_height + cover_internal_height, cover_internal_corner_radius2);
//
//    //translate([6, cover_y + 26, 7]) rotate([0, 180, 0]) scale([1.3, 1.3, 3]) import("/Users/tal/Downloads/_prusa/rina_text.stl", convexity=3);
//  }
//
//}

//if (part_selector == 1 || part_selector == 3) {
main();

//cut();
//} 
//
//if (part_selector == 2 || part_selector == 3) {
//  cover();
//} 

//translate([16, 31, 7]) rotate([0, 180, 0]) scale([1.3, 1.3, 3]) import("/Users/tal/Downloads/_prusa/rina_text.stl", convexity=3);






