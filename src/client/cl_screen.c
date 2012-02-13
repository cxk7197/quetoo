/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "cl_local.h"

#define NET_GRAPH_HEIGHT 64
#define NET_GRAPH_WIDTH 128
#define NET_GRAPH_Y 0

// net graph samples
typedef struct {
	float value;
	int color;
} net_graph_sample_t;

static net_graph_sample_t net_graph_samples[NET_GRAPH_WIDTH];
static int num_net_graph_samples;

/*
 * Cl_NetGraph
 */
static void Cl_NetGraph(float value, int color) {

	net_graph_samples[num_net_graph_samples].value = value;
	net_graph_samples[num_net_graph_samples].color = color;

	if (net_graph_samples[num_net_graph_samples].value > 1.0)
		net_graph_samples[num_net_graph_samples].value = 1.0;

	num_net_graph_samples++;

	if (num_net_graph_samples == NET_GRAPH_WIDTH)
		num_net_graph_samples = 0;
}

/*
 * Cl_AddNetGraph
 */
void Cl_AddNetGraph(void) {
	unsigned int i;
	unsigned int in;
	unsigned int ping;

	// we only need to do our accounting when asked to
	if (!cl_draw_net_graph->value)
		return;

	for (i = 0; i < cls.netchan.dropped; i++)
		Cl_NetGraph(1.0, 0x40);

	for (i = 0; i < cl.surpress_count; i++)
		Cl_NetGraph(1.0, 0xdf);

	// see what the latency was on this packet
	in = cls.netchan.incoming_acknowledged & (CMD_BACKUP - 1);
	ping = cls.real_time - cl.cmd_time[in];

	Cl_NetGraph(ping / 300.0, 0xd0); // 300ms is lagged out
}

/*
 * Cl_DrawNetGraph
 */
static void Cl_DrawNetGraph(void) {
	int i, j, x, y, h;

	if (!cl_draw_net_graph->value)
		return;

	x = r_context.width - NET_GRAPH_WIDTH;
	y = r_context.height - NET_GRAPH_Y - NET_GRAPH_HEIGHT;

	R_DrawFill(x, y, NET_GRAPH_WIDTH, NET_GRAPH_HEIGHT, 8, 0.2);

	for (i = 0; i < NET_GRAPH_WIDTH; i++) {

		j = (num_net_graph_samples - i) & (NET_GRAPH_WIDTH - 1);
		h = net_graph_samples[j].value * NET_GRAPH_HEIGHT;

		if (!h)
			continue;

		x = r_context.width - i;
		y = r_context.height - NET_GRAPH_Y - h;

		R_DrawFill(x, y, 1, h, net_graph_samples[j].color, 0.5);
	}
}

/*
 * Cl_DrawRendererStats
 */
static void Cl_DrawRendererStats(void) {
	char s[128];

	if (!cl_show_renderer_stats->value)
		return;

	if (cls.state != CL_ACTIVE)
		return;

	snprintf(s, sizeof(s) - 1, "%i bsp %i mesh %i lights %i particles",
			r_view.bsp_polys, r_view.mesh_polys, r_view.num_lights, r_view.num_particles);

	R_DrawString(r_context.width - strlen(s) * 16, 0, s, CON_COLOR_YELLOW);
}

unsigned short frames_this_second = 0, packets_this_second = 0, bytes_this_second = 0;

/*
 * Cl_DrawCounters
 */
static void Cl_DrawCounters(void) {
	static vec3_t velocity;
	static char bps[8], pps[8], fps[8], spd[8];
	static int millis;
	r_pixel_t cw, ch;

	if (!cl_draw_counters->value)
		return;

	R_BindFont("small", &cw, &ch);

	const r_pixel_t x = r_context.width - 7 * cw;
	r_pixel_t y = r_context.height - 4 * ch;

	frames_this_second++;

	if (cls.real_time - millis >= 1000) {

		VectorCopy(r_view.velocity, velocity);
		velocity[2] = 0.0;

		snprintf(spd, sizeof(spd), "%4.0fspd", VectorLength(velocity));
		snprintf(fps, sizeof(fps), "%4ufps", frames_this_second);
		snprintf(pps, sizeof(pps), "%4upps", packets_this_second);
		snprintf(bps, sizeof(bps), "%4ubps", bytes_this_second);

		millis = quake2world.time;

		frames_this_second = 0;
		packets_this_second = 0;
		bytes_this_second = 0;
	}

	R_DrawString(x, y, spd, CON_COLOR_DEFAULT);
	y += ch;

	R_DrawString(x, y, fps, CON_COLOR_DEFAULT);
	y += ch;

	R_DrawString(x, y, pps, CON_COLOR_DEFAULT);
	y += ch;

	R_DrawString(x, y, bps, CON_COLOR_DEFAULT);

	R_BindFont(NULL, NULL, NULL);
}

/*
 * Cl_DrawCursor
 */
static void Cl_DrawCursor(void) {

	if (cls.key_state.dest != KEY_UI && cls.mouse_state.grabbed)
		return;

	if (!(SDL_GetAppState() & SDL_APPMOUSEFOCUS))
		return;

	R_DrawCursor(cls.mouse_state.x, cls.mouse_state.y);
}

/*
 * Cl_UpdateScreen
 *
 * This is called every frame, and can also be called explicitly to flush
 * text to the screen.
 */
void Cl_UpdateScreen(void) {

	R_BeginFrame();

	if (cls.state == CL_ACTIVE && !cls.loading) {

		Cl_UpdateView();

		R_Setup3D();

		R_DrawScene();

		R_Setup2D();

		if (cls.key_state.dest != KEY_CONSOLE && cls.key_state.dest != KEY_UI) {

			Cl_DrawNetGraph();

			Cl_DrawCounters();

			Cl_DrawNotify();

			Cl_DrawRendererStats();

			cls.cgame->DrawFrame(&cl.frame);
		}
	} else {
		R_Setup2D();
	}

	Cl_DrawConsole();

	R_DrawFills(); // draw all fills accumulated above

	R_DrawLines(); // draw all lines accumulated above

	R_DrawChars(); // draw all chars accumulated above

	Ui_Draw();

	Cl_DrawCursor();

	R_EndFrame();
}
