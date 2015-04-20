#include <pebble.h>
#include "gpath_builder.h"
#include "draw_layers.h"

/********************************************/
/*************** DECLARATIONS ***************/
/********************************************/
	
/* declaration of variable "objects" */
static Window *graphicWindow, *detailWindow;
static TextLayer *graphicHeader[2], *detailHeader, *detailText;
static Layer *actionBarLayer[2];
static Layer *actionBarIconGraphic[3];
static Layer *actionBarIconDetail;
static Layer *graphicDrawLayer[2];
static Layer *graphicBackgroundLayer;
static PropertyAnimation *animations[3];

/* declaration of variables */
static int active = 0;
static int drawingItem[2];

/* declaration of constants with #define statements */
#define BG_COLOUR GColorDarkGray
#define TEXT_COLOUR GColorWhite
#define BAR_BG_COLOUR GColorBlack
#define ICON_COLOUR GColorWhite
#define SCREEN_HEIGHT 168
#define HEADER_HEIGHT 27
#define DETAIL_OFFSET 3
#define DETAIL_SPACE 5
#define BAR_WIDTH 15
#define BAR_SPACE 10
#define BAR_ROUNDING 3
#define TRIANGLE_BASE 11
#define TRIANGLE_HEIGHT 6
#define ICON_SPACE 7
#define HEADER_WIDTH 144 - BAR_WIDTH
#define LAYER 0
#define HEADER 1
#define BACKGROUND 2
#define ANIMATION_SPEED 500
#define HEADER_FONT FONT_KEY_GOTHIC_24_BOLD
#define DETAIL_FONT FONT_KEY_GOTHIC_18

/* declarations for functions which are implemented below (for improved code legibility) */
static TextLayer* get_header_layer();
static Layer* get_action_bar_layer(Window *window);							// not a real action bar
static Layer* get_icon_layer(Window *window, int position);
static void draw_action_bar_round_rect(Layer *l, GContext *ctx);
static void draw_arrow_up(Layer *l, GContext *ctx);
static void draw_arrow_down(Layer *l, GContext *ctx);
static void draw_arrow_left(Layer *l, GContext *ctx);
static void draw_arrow_right(Layer *l, GContext *ctx);
static void format_header_layer(TextLayer *t);
static void detail_window_push();
static void detail_window_pop();
static void update_layer_1_proc(Layer *l, GContext *ctx);
static void update_layer_2_proc(Layer *l, GContext *ctx);
static void move_layer_above_screen(Layer *l);
static void move_layer_below_screen(Layer *l);
static void background_update_proc(Layer *l, GContext *ctx);

/********************************************/
/***** CLICK HANDLERS FOR DETAIL WINDOW *****/
/********************************************/

/* click handler for the detail window, select button - call the pop routine */
static void detail_window_select_handler(ClickRecognizerRef recogniser, void *context) {
	detail_window_pop();
}

/* click handler for the detail window, back button - call the pop routine */
static void detail_window_back_handler(ClickRecognizerRef recogniser, void *context) {
	detail_window_pop();
}

/* click config for the detail window */
static void detail_window_click_config(void *data) {
	window_single_click_subscribe(BUTTON_ID_SELECT, detail_window_select_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, detail_window_back_handler);
}

/* pop the detail window off the stack, revealing graphic window */
static void detail_window_pop() {
	window_stack_pop(true);
	window_destroy(detailWindow);
}

/********************************************/
/**** CLICK HANDLERS FOR GRAPHIC WINDOW *****/
/********************************************/

/* called when animations end - remove "covered" layers and switch active */
static void layer_animation_ended(Animation *animation, bool finished, void *data) {
	/* we assume at this point that our animations are ALL done */
	layer_remove_from_parent(graphicDrawLayer[active]);
	layer_remove_from_parent(text_layer_get_layer(graphicHeader[active]));
	layer_remove_from_parent(graphicBackgroundLayer);
	
	/* switch active */
	active = 1 - active;
}

/* preps for animations and schedules them */
static void set_for_animation() {
	/* calculate inactive layer */
	int inactive = 1 - active;
	
	/* add layers to window */
	Layer *w = window_get_root_layer(graphicWindow);
	layer_add_child(w, graphicBackgroundLayer);
	layer_add_child(w, graphicDrawLayer[inactive]);
	layer_add_child(w, text_layer_get_layer(graphicHeader[inactive]));
	
	/* set the 'to' frame for background layer */
	int width = layer_get_frame(w).size.w - BAR_WIDTH;
	int height = layer_get_frame(w).size.h;
	GRect backgroundTo = GRect(0,0,width,height);
	
	/* set the 'to' frame for draw layer */
	width = layer_get_frame(w).size.w - BAR_WIDTH - 2 * DETAIL_OFFSET;
	height = layer_get_frame(w).size.h - HEADER_HEIGHT - DETAIL_SPACE - DETAIL_OFFSET;
	GRect layerTo = GRect(DETAIL_OFFSET, HEADER_HEIGHT + DETAIL_SPACE, width, height);
	
	/* set the 'to' frame for header*/
	GRect headerTo = GRect(0,0,HEADER_WIDTH,HEADER_HEIGHT);
	
	/* set layer frames */
	animations[BACKGROUND] = property_animation_create_layer_frame(graphicBackgroundLayer, NULL, &backgroundTo);
	animations[LAYER] = property_animation_create_layer_frame(graphicDrawLayer[inactive], NULL, &layerTo);
	animations[HEADER] = property_animation_create_layer_frame(text_layer_get_layer(graphicHeader[inactive]), NULL, &headerTo);
	
	/* set timing */
	animation_set_duration((Animation *)animations[BACKGROUND], ANIMATION_SPEED);
	animation_set_duration((Animation *)animations[LAYER], ANIMATION_SPEED);
	animation_set_duration((Animation *)animations[HEADER], ANIMATION_SPEED);
	
	/* set handler ONLY for layer */
	animation_set_handlers((Animation *) animations[LAYER], (AnimationHandlers) {
		.started = (AnimationStartedHandler) NULL,
		.stopped = (AnimationStoppedHandler) layer_animation_ended,
	}, NULL);
	
	/* scheduled animations */
	animation_schedule((Animation *)animations[BACKGROUND]);
	animation_schedule((Animation *)animations[LAYER]);
	animation_schedule((Animation *)animations[HEADER]);
}

/* specific preparation after pushing the "up" button before handing off to set_for_animation */
static void push_graphic_window_up() {
	/* calculate inactive layer */
	int inactive = 1 - active;
	
	/* get number of next item up and assign */
	drawingItem[inactive] = next_up(drawingItem[active]);
	
	/* change the text on the header */
	text_layer_set_text(graphicHeader[inactive], header_text(drawingItem[inactive]));
	
	/* move inactive text layer and layer frames to be off screen */
	move_layer_below_screen(graphicDrawLayer[inactive]);
	move_layer_below_screen(text_layer_get_layer(graphicHeader[inactive]));
	move_layer_below_screen(graphicBackgroundLayer);
	
	/* get set for animation */
	set_for_animation();
}

/* specific preparation after pushing the "down" button before handing off to set_for_animation */
static void push_graphic_window_down() {
	/* calculate inactive layer */
	int inactive = 1 - active;
	
	/* get number of next item down and assign */
	drawingItem[inactive] = next_down(drawingItem[active]);
	
	/* change the text on the header */
	text_layer_set_text(graphicHeader[inactive], header_text(drawingItem[inactive]));
	
	/* move inactive text layer and layer frames to be off screen */
	move_layer_above_screen(graphicDrawLayer[inactive]);
	move_layer_above_screen(text_layer_get_layer(graphicHeader[inactive]));
	move_layer_above_screen(graphicBackgroundLayer);
	
	/* get set for animation */
	set_for_animation();
}

/* graphic window select handler - call "detail window push" */
static void graphic_window_select_handler(ClickRecognizerRef recogniser, void *context) {
	detail_window_push();
}

/* graphic window click config provider */
static void graphic_window_click_config(void *data) {
	window_single_click_subscribe(BUTTON_ID_SELECT, graphic_window_select_handler);
	window_single_click_subscribe(BUTTON_ID_UP, push_graphic_window_up);
	window_single_click_subscribe(BUTTON_ID_DOWN, push_graphic_window_down);
}

/********************************************/
/********** DETAIL WINDOW HANDLERS **********/
/********************************************/

/* detail window load handler */
static void detail_window_load(Window *window) {
	/* get the window root layer as layer - worth it because adding multiple children */
	Layer *w = window_get_root_layer(window);
	
	/* set background colour */
	window_set_background_color(window, BG_COLOUR);
	
	/* get new (formatted) header layer, set text, add to window*/
	detailHeader = get_header_layer();
	text_layer_set_text(detailHeader, header_text(drawingItem[active]));
	layer_add_child(w, text_layer_get_layer(detailHeader));
	
	/* make detailed textLayer - will abstract this later */
	int width = layer_get_frame(w).size.w - BAR_WIDTH - 2 * DETAIL_OFFSET;
	int height = layer_get_frame(w).size.h - HEADER_HEIGHT - DETAIL_SPACE - DETAIL_OFFSET;
	detailText = text_layer_create(GRect(DETAIL_OFFSET, HEADER_HEIGHT + DETAIL_SPACE, width, height));
	text_layer_set_text(detailText, detail_text(drawingItem[active]));
	text_layer_set_background_color(detailText, GColorClear);
	text_layer_set_text_color(detailText, TEXT_COLOUR);
	text_layer_set_font(detailText, fonts_get_system_font(DETAIL_FONT));
	layer_add_child(w, text_layer_get_layer(detailText));
	
	/* add the 'faux' action bar */
	actionBarLayer[1] = get_action_bar_layer(window);
	layer_add_child(w, actionBarLayer[1]);
	
	/* add the arrow icon */
	actionBarIconDetail = get_icon_layer(window, 1);
	layer_set_update_proc(actionBarIconDetail, draw_arrow_left);
	layer_add_child(w, actionBarIconDetail);
	
	/* add click config to window */
	window_set_click_config_provider(window, (ClickConfigProvider)detail_window_click_config);
}

/* detail window unload handler */
static void detail_window_unload(Window *window) {
	text_layer_destroy(detailHeader);
	text_layer_destroy(detailText);
	layer_destroy(actionBarLayer[1]);
	layer_destroy(actionBarIconDetail);
}

/* detail window push - create detail window, set handlers, push to stack */
static void detail_window_push() {
	detailWindow = window_create();
	window_set_window_handlers(detailWindow, (WindowHandlers) {
		.load = detail_window_load,
		.unload = detail_window_unload,
	});
	window_stack_push(detailWindow, true);
}

/********************************************/
/********** GRAPHIC WINDOW HANDLERS *********/
/********************************************/

/* graphic window load handler */
static void graphic_window_load(Window *window) {
	/* get the window root layer as layer - worth it because adding multiple children */
	Layer *w = window_get_root_layer(window);
	
	/* set background colour */
	window_set_background_color(window, BG_COLOUR);
	
	/* get new (formatted) header layers, set text, add the base one to the window */
	graphicHeader[0] = get_header_layer();
	graphicHeader[1] = get_header_layer();
	text_layer_set_text(graphicHeader[active], header_text(drawingItem[active]));
	layer_add_child(w, text_layer_get_layer(graphicHeader[active]));
	
	/* add the 'faux' action bar */
	actionBarLayer[0] = get_action_bar_layer(window);
	layer_add_child(w, actionBarLayer[0]);
	
	/* add the arrow icons */
	for (int i = 0; i < 3; i++) {
		actionBarIconGraphic[i] = get_icon_layer(window, i);
	}
	layer_set_update_proc(actionBarIconGraphic[0], draw_arrow_up);
	layer_set_update_proc(actionBarIconGraphic[1], draw_arrow_right);
	layer_set_update_proc(actionBarIconGraphic[2], draw_arrow_down);
	for (int i = 0; i < 3; i++) {
		layer_add_child(w, actionBarIconGraphic[i]);
	}
	
	/* make both draw layers, add the active one */
	int width = layer_get_frame(w).size.w - BAR_WIDTH - 2 * DETAIL_OFFSET;
	int height = layer_get_frame(w).size.h - HEADER_HEIGHT - DETAIL_SPACE - DETAIL_OFFSET;
	graphicDrawLayer[0] = layer_create(GRect(DETAIL_OFFSET, HEADER_HEIGHT + DETAIL_SPACE, width, height));
	graphicDrawLayer[1] = layer_create(GRect(DETAIL_OFFSET, HEADER_HEIGHT + DETAIL_SPACE, width, height));
	layer_set_update_proc(graphicDrawLayer[0], update_layer_1_proc);
	layer_set_update_proc(graphicDrawLayer[1], update_layer_2_proc);
	layer_add_child(w, graphicDrawLayer[active]);
	
	/* make (but don't add) the graphic background layer */
	width = layer_get_frame(w).size.w - BAR_WIDTH;
	height = layer_get_frame(w).size.h;
	graphicBackgroundLayer = layer_create(GRect(0,0,width,height));
	layer_set_update_proc(graphicBackgroundLayer, background_update_proc);
	
	/* set click config for window */
	window_set_click_config_provider(window, (ClickConfigProvider)graphic_window_click_config);
}

/* graphic window unload handler */
static void graphic_window_unload(Window *window) {
	for (int i = 0; i < 2; i++) {
		text_layer_destroy(graphicHeader[i]);
		layer_destroy(graphicDrawLayer[i]);
	}
	layer_destroy(actionBarLayer[0]);
	for (int i = 0; i < 3; i++) {
		layer_destroy(actionBarIconGraphic[i]);
	}
	layer_destroy(graphicBackgroundLayer);
}

/********************************************/
/************ MAIN, INIT & DEINIT ***********/
/********************************************/

static void init(void) {
  graphicWindow = window_create();
  window_set_window_handlers(graphicWindow, (WindowHandlers) {
    .load = graphic_window_load,
    .unload = graphic_window_unload,
  });
  window_stack_push(graphicWindow, true);
}

static void deinit(void) {
	window_destroy(graphicWindow);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

/**********************************************/
/******* HELPER METHODS - HEADER LAYERS *******/
/**********************************************/

/* return a header layer, sized and formatted*/
static TextLayer* get_header_layer() {
	TextLayer *temp = text_layer_create(GRect(0,0,HEADER_WIDTH,HEADER_HEIGHT));
	format_header_layer(temp);
	return temp;
}

/* formatting header layer */
static void format_header_layer(TextLayer *t) {
	text_layer_set_text_alignment(t, GTextAlignmentCenter);
	text_layer_set_background_color(t, GColorClear);
	text_layer_set_text_color(t, TEXT_COLOUR);
	text_layer_set_font(t, fonts_get_system_font(HEADER_FONT));
}

/**********************************************/
/****** HELPER METHODS - FAUX ACTION BAR ******/
/**********************************************/

/* return a 'faux' action bar layer - rounded rect */
static Layer* get_action_bar_layer(Window *window) {
	Layer *windowLayer = window_get_root_layer(window);
	int width = layer_get_frame(windowLayer).size.w;
	int height = layer_get_frame(windowLayer).size.h;
	Layer *temp = layer_create(GRect(width - BAR_WIDTH, BAR_SPACE, BAR_WIDTH, height - 2 * BAR_SPACE));
	layer_set_update_proc(temp, draw_action_bar_round_rect);
	return temp;
}

/* update proc to draw rounded rect in the 'faux' action bar */
static void draw_action_bar_round_rect(Layer *layer, GContext *ctx) {
	int width = layer_get_frame(layer).size.w;
	int height = layer_get_frame(layer).size.h;
	graphics_context_set_fill_color(ctx, BAR_BG_COLOUR);
	graphics_fill_rect(ctx, GRect(0,0,width,height), BAR_ROUNDING, GCornersLeft);
}

/**********************************************/
/**** HELPER METHODS - ICONS IN ACTION BAR ****/
/**********************************************/

/* draw up arrow as icon in the 'faux' action bar */
static void draw_arrow_up(Layer *layer, GContext *ctx) {
	int height = layer_get_frame(layer).size.h;
	int width = layer_get_frame(layer).size.w;
	GPathInfo tempPathInfo = {
		.num_points = 3,
		.points = (GPoint[]) { {width/2 , 0}, {width, height}, {0, height} }
	};
	GPath *tempPath = gpath_create(&tempPathInfo);
	graphics_context_set_fill_color(ctx, ICON_COLOUR);
	gpath_draw_filled(ctx, tempPath);
	gpath_destroy(tempPath);
}

/* draw down arrow as icon in the 'faux' action bar */
static void draw_arrow_down(Layer *layer, GContext *ctx) {
	int height = layer_get_frame(layer).size.h;
	int width = layer_get_frame(layer).size.w;
	GPathInfo tempPathInfo = {
		.num_points = 3,
		.points = (GPoint[]) { {width/2 , height}, {0, 0}, {width, 0} }
	};
	GPath *tempPath = gpath_create(&tempPathInfo);
	graphics_context_set_fill_color(ctx, ICON_COLOUR);
	gpath_draw_filled(ctx, tempPath);
	gpath_destroy(tempPath);
}

/* draw right arrow as icon in the 'faux' action bar */
static void draw_arrow_right(Layer *layer, GContext *ctx) {
	int height = layer_get_frame(layer).size.h;
	int width = layer_get_frame(layer).size.w;
	GPathInfo tempPathInfo = {
		.num_points = 3,
		.points = (GPoint[]) { {width, height/2 + 1}, {0, height}, {0, 0} }
	};
	GPath *tempPath = gpath_create(&tempPathInfo);
	graphics_context_set_fill_color(ctx, ICON_COLOUR);
	gpath_draw_filled(ctx, tempPath);
	gpath_destroy(tempPath);
}

/* draw left arrow as icon in the 'faux' action bar */
static void draw_arrow_left(Layer *layer, GContext *ctx) {
	int height = layer_get_frame(layer).size.h;
	int width = layer_get_frame(layer).size.w;
	GPathInfo tempPathInfo = {
		.num_points = 3,
		.points = (GPoint[]) { {0 , height/2}, {width,0}, {width, height} }
	};
	GPath *tempPath = gpath_create(&tempPathInfo);
	graphics_context_set_fill_color(ctx, ICON_COLOUR);
	gpath_draw_filled(ctx, tempPath);
	gpath_destroy(tempPath);
}

/* returns a layer for an icon on the 'faux' action bar, sized and positioned */
static Layer* get_icon_layer(Window *window, int position) {
	int xPos, yPos, width, height;
	Layer *windowLayer = window_get_root_layer(window);
	switch(position) {
		case 0:
			yPos = BAR_SPACE + ICON_SPACE;
			width = TRIANGLE_BASE;
			height = TRIANGLE_HEIGHT;
			break;
		case 1:
			yPos = (layer_get_frame(windowLayer).size.h - TRIANGLE_BASE) / 2;
			width = TRIANGLE_HEIGHT;
			height = TRIANGLE_BASE;
			break;
		case 2:
			yPos = layer_get_frame(windowLayer).size.h - BAR_SPACE - ICON_SPACE - TRIANGLE_HEIGHT;
			width = TRIANGLE_BASE;
			height = TRIANGLE_HEIGHT;
			break;
		default:
			xPos = 0;
			yPos = 0;
			width = 0;
			height = 0;
			break;
	}
	xPos = layer_get_frame(windowLayer).size.w - BAR_WIDTH + (BAR_WIDTH - width)/2;
	Layer *temp = layer_create(GRect(xPos, yPos, width, height));
	return temp;
}

/**********************************************/
/*** HELPER METHODS - BACKGROUND DRAW PROC ****/
/**********************************************/

/* graphicBackgroundLayer is always a rect in the background colour to mask active layers */
static void background_update_proc(Layer *l, GContext *ctx) {
	int width = layer_get_frame(l).size.w;
	int height = layer_get_frame(l).size.h;
	graphics_context_set_fill_color(ctx, BG_COLOUR);
	graphics_fill_rect(ctx, GRect(0,0,width,height), 0, GCornerNone);
}

/**********************************************/
/*** HELPER METHODS - POSITIONS FOR LAYERS ****/
/**********************************************/

/* change a layer's frame so it's below the screen (origin.y is >= 168) */
static void move_layer_below_screen(Layer *l) {
	int xPos = layer_get_frame(l).origin.x;
	int yPos = layer_get_frame(l).origin.y;
	int width = layer_get_frame(l).size.w;
	int height = layer_get_frame(l).size.h;
	while(yPos < SCREEN_HEIGHT) {
		yPos += SCREEN_HEIGHT;
	}
	layer_set_frame(l, GRect(xPos,yPos,width,height));
}

/* change a layer's frame so it's above the screen (origin.y is < 0) */
static void move_layer_above_screen(Layer *l) {
	int xPos = layer_get_frame(l).origin.x;
	int yPos = layer_get_frame(l).origin.y;
	int width = layer_get_frame(l).size.w;
	int height = layer_get_frame(l).size.h;
	while(yPos >= 0) {
		yPos -= SCREEN_HEIGHT;
	}
	layer_set_frame(l, GRect(xPos,yPos,width,height));	
}

/**********************************************/
/***** HELPER METHODS - TEMP LAYER DRAWING ****/
/**********************************************/

static void update_layer_1_proc(Layer *l, GContext *ctx) {
	draw_graphics_image(drawingItem[0], ctx);
}

static void update_layer_2_proc(Layer *l, GContext *ctx) {
	draw_graphics_image(drawingItem[1],ctx);
}