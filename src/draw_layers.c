#include <pebble.h>
#include "draw_layers.h"
#include "gpath_builder.h"

/********************************************/
/*************** DECLARATIONS ***************/
/********************************************/
	
#define ENTRIES 6
#define MAX_POINTS 256
#define CUP_STROKE 3
#define HANDLE_STROKE 3
#define LIM_STROKE 6
#define OUTLINE_STROKE 2
	
#define CUP_COLOUR GColorWhite
#define COFFEE_COLOUR GColorBlack
#define LIM_COLOUR GColorDarkGray
#define OUTLINE_COLOUR GColorDarkGray
#define FOAM_COLOUR GColorPastelYellow
#define WATER_COLOUR GColorBabyBlueEyes
#define MILK_COLOUR GColorWhite

char *headerText[ENTRIES];
char *detailText[ENTRIES];
bool headersPopulated = false;
bool detailsPopulated = false;

/* empty function declarations so we can put them below for improved legibility */
void draw_espresso(GContext *ctx);
void draw_americano(GContext *ctx);
void draw_cappuccino(GContext *ctx);
void draw_latte(GContext *ctx);
void draw_macchiato(GContext *ctx);
void draw_ristretto(GContext *ctx);

void draw_cup(GContext *ctx);
void draw_handle(GContext *ctx);
void draw_espresso_shot(GContext *ctx);
void draw_foam_to_top(GContext *ctx);
void draw_water_to_top(GContext *ctx);
void draw_milk_to_mid(GContext *ctx);
void draw_milk_to_high(GContext *ctx);
void draw_milk_to_low(GContext *ctx);
void draw_foam_to_very_low(GContext *ctx);

/********************************************/
/**** HELPER METHODS TO POPULATE ARRAYS *****/
/********************************************/

/* populates the headerText array */
void setup_header_text_array() {
	headerText[0] = "Espresso";
	headerText[1] = "Americano";
	headerText[2] = "Cappuccino";
	headerText[3] = "Latte";
	headerText[4] = "Macchiato";
	headerText[5] = "Ristretto";
}

/* populates the detailText array */
void setup_detail_text_array() {
	detailText[0] = "Espresso is made by forcing hot water through finely ground coffee at high pressure";
	detailText[1] = "An espresso shot topped up with hot water (black americano) and maybe some milk (white americano)";
	detailText[2] = "An espresso shot topped up with equal amounts of steamed milk and milk froth";
	detailText[3] = "An espresso shot topped with steam milk - similar to cappuccino but without the froth";
	detailText[4] = "An espresso shot 'stained' with a little steamed or frothed milk";
	detailText[5] = "A 'short' shot of espresso, giving a bolder flavour with less bitterness";
}

/********************************************/
/***** METHODS TO RETURN REQUESTED TEXT *****/
/********************************************/

/* return the requested entry from headerText array */
char* header_text(int i) {
	/* check whether we've populated the array, if not then do so */
	if(!headersPopulated) {
		setup_header_text_array();
		headersPopulated = true;
	}
	/* return the relevant header */
	return headerText[i];
}

/* return the requested entry from detailText array */
char* detail_text(int i) {
	/* check whether we've populated the array, if not then do so */
	if(!detailsPopulated) {
		setup_detail_text_array();
		detailsPopulated = true;
	}
	/* return the relevant header */
	return detailText[i];
}

/********************************************/
/***** METHODS TO GET NEXT ENTRY NUMBER *****/
/********************************************/

/* get the next entry down, cycling up to end if we're at start */
int next_down(int current) {
	int i = (current == (ENTRIES-1)) ? 0 : current + 1;
	return i;
}

/* get the next entry up, cycling down to start if we're at end */
int next_up(int current) {
	int i = (current == 0) ? ENTRIES - 1 : current - 1;
	return i;
}

/********************************************/
/***** IMAGE DRAWING - MAIN SWITCH CALL *****/
/********************************************/

/* main image switcher with a casecading if/else statement */
void draw_graphics_image(int i, GContext *ctx) {
	if (i >= ENTRIES) {
		return;
	} else if (i == 0) {
		draw_espresso(ctx);
	} else if (i == 1) {
		draw_americano(ctx);
	} else if (i == 2) {
		draw_cappuccino(ctx);
	} else if (i == 3) {
		draw_latte(ctx);
	} else if (i == 4) {
		draw_macchiato(ctx);
	} else if (i == 5) {
		draw_ristretto(ctx);
	}
}

/********************************************/
/**** IMAGES - INDIVIDUAL DRAW FUNCTIONS ****/
/********************************************/

void draw_espresso(GContext *ctx) {
	draw_espresso_shot(ctx);
	draw_cup(ctx);
	draw_handle(ctx);
}

void draw_americano(GContext *ctx) {
	draw_water_to_top(ctx);
	draw_espresso_shot(ctx);
	draw_cup(ctx);
	draw_handle(ctx);
}

void draw_cappuccino(GContext *ctx) {
	draw_foam_to_top(ctx);
	draw_milk_to_mid(ctx);
	draw_espresso_shot(ctx);
	draw_cup(ctx);
	draw_handle(ctx);
}

void draw_latte(GContext *ctx) {
	draw_foam_to_top(ctx);
	draw_milk_to_high(ctx);
	draw_espresso_shot(ctx);
	draw_cup(ctx);
	draw_handle(ctx);
}

void draw_macchiato(GContext *ctx) {
	draw_foam_to_very_low(ctx);
	draw_espresso_shot(ctx);
	draw_cup(ctx);
	draw_handle(ctx);
}

void draw_ristretto(GContext *ctx) {
	draw_espresso_shot(ctx);
	draw_cup(ctx);
	draw_handle(ctx);
}

/********************************************/
/***** IMAGES - DETAILED DRAW FUNCTIONS *****/
/********************************************/

/* draw the cup */
void draw_cup(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(5,20));
	gpath_builder_line_to_point(builder, GPoint(115,20));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(115,60), GPoint(90,100));
	gpath_builder_curve_to_point(builder, GPoint(5,20), GPoint(30,100), GPoint(5,60));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);

	/* prepare context & stroke path - outline */
	graphics_context_set_stroke_color(ctx, LIM_COLOUR);
	graphics_context_set_stroke_width(ctx, CUP_STROKE + LIM_STROKE);
	gpath_draw_outline(ctx, temp);
	
	/* prepare context & stroke path - 'fill' */
	graphics_context_set_stroke_color(ctx, CUP_COLOUR);
	graphics_context_set_stroke_width(ctx, CUP_STROKE);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/* draw the handle */
void draw_handle(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(114,30));
	gpath_builder_curve_to_point(builder, GPoint(122,38), GPoint(120,25), GPoint(123,25));
	gpath_builder_curve_to_point(builder, GPoint(111,45), GPoint(120,50), GPoint(110,47));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context & stroke path - 'fill' */
	graphics_context_set_stroke_color(ctx, CUP_COLOUR);
	graphics_context_set_stroke_width(ctx, HANDLE_STROKE);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/* draw an espresso shot - also used for ristretto despite it supposedly being a little shorter */
void draw_espresso_shot(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(20,75));
	gpath_builder_line_to_point(builder, GPoint(100,75));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(95,85), GPoint(75,100));
	gpath_builder_curve_to_point(builder, GPoint(20,75), GPoint(45,100), GPoint(25,85));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context */
	graphics_context_set_stroke_color(ctx, OUTLINE_COLOUR);
	graphics_context_set_stroke_width(ctx, OUTLINE_STROKE);
	graphics_context_set_fill_color(ctx, COFFEE_COLOUR);
	
	/* stroke path then fill path */
	gpath_draw_filled(ctx, temp);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/* helper routine for drawing foam and milke to top */
void draw_to_top(GContext *ctx, GColor color) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(8,35));
	gpath_builder_line_to_point(builder, GPoint(110,35));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(110,70), GPoint(80,100));
	gpath_builder_curve_to_point(builder, GPoint(8,35), GPoint(40,100), GPoint(8,70));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context */
	graphics_context_set_stroke_color(ctx, OUTLINE_COLOUR);
	graphics_context_set_stroke_width(ctx, OUTLINE_STROKE);
	graphics_context_set_fill_color(ctx, color);
	
	/* stroke path then fill path */
	gpath_draw_filled(ctx, temp);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/* draw foam to top, using helper function */
void draw_foam_to_top(GContext *ctx) {
	draw_to_top(ctx, FOAM_COLOUR);
}

/* draw water to top, using helper function */
void draw_water_to_top(GContext *ctx) {
	draw_to_top(ctx, WATER_COLOUR);
}

/* draw milk half way */
void draw_milk_to_mid(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(15,55));
	gpath_builder_line_to_point(builder, GPoint(105,55));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(105,75), GPoint(80,100));
	gpath_builder_curve_to_point(builder, GPoint(15,55), GPoint(40,100), GPoint(15,75));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context */
	graphics_context_set_stroke_color(ctx, OUTLINE_COLOUR);
	graphics_context_set_stroke_width(ctx, OUTLINE_STROKE);
	graphics_context_set_fill_color(ctx, MILK_COLOUR);
	
	/* stroke path then fill path */
	gpath_draw_filled(ctx, temp);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/* draw milk nearly to the top */
void draw_milk_to_high(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(10,42));
	gpath_builder_line_to_point(builder, GPoint(110,42));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(110,70), GPoint(80,100));
	gpath_builder_curve_to_point(builder, GPoint(10,42), GPoint(40,100), GPoint(10,70));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context */
	graphics_context_set_stroke_color(ctx, OUTLINE_COLOUR);
	graphics_context_set_stroke_width(ctx, OUTLINE_STROKE);
	graphics_context_set_fill_color(ctx, MILK_COLOUR);
	
	/* stroke path then fill path */
	gpath_draw_filled(ctx, temp);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/***** NOT CURRENTLY USED *****/
void draw_milk_to_low(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(12,42));
	gpath_builder_line_to_point(builder, GPoint(108,42));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(108,70), GPoint(80,100));
	gpath_builder_curve_to_point(builder, GPoint(12,42), GPoint(40,100), GPoint(12,70));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context */
	graphics_context_set_stroke_color(ctx, OUTLINE_COLOUR);
	graphics_context_set_stroke_width(ctx, OUTLINE_STROKE);
	graphics_context_set_fill_color(ctx, MILK_COLOUR);
	
	/* stroke path then fill path */
	gpath_draw_filled(ctx, temp);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}

/* draw foam just above the espresso shot */
void draw_foam_to_very_low(GContext *ctx) {
	/* make an epmty GPathBuilder */
	GPathBuilder *builder = gpath_builder_create(MAX_POINTS);
	
	/* build the path */
	gpath_builder_move_to_point(builder, GPoint(18,67));
	gpath_builder_line_to_point(builder, GPoint(102,67));
	gpath_builder_curve_to_point(builder, GPoint(60,100), GPoint(102,85), GPoint(75,100));
	gpath_builder_curve_to_point(builder, GPoint(18,67), GPoint(40,100), GPoint(18,85));
	
	/* convert to a GPath */
	GPath *temp = gpath_builder_create_path(builder);
	
	/* prepare context */
	graphics_context_set_stroke_color(ctx, OUTLINE_COLOUR);
	graphics_context_set_stroke_width(ctx, OUTLINE_STROKE);
	graphics_context_set_fill_color(ctx, FOAM_COLOUR);
	
	/* stroke path then fill path */
	gpath_draw_filled(ctx, temp);
	gpath_draw_outline(ctx, temp);
	
	/* destroy objects */
	gpath_builder_destroy(builder);
	gpath_destroy(temp);
}