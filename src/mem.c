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

#include "mem.h"

#include <glib.h>
#include <SDL/SDL_thread.h>

#define Z_MAGIC 0x69
typedef byte z_magic_t;

typedef struct z_block_s {
	z_magic_t magic;
	z_tag_t tag; // for group free
	struct z_block_s *parent;
	GList *children;
} z_block_t;

typedef struct {
	GList *blocks;
	SDL_mutex *lock;
} z_state_t;

static z_state_t z_state;

/*
 * @brief Performs the actual grunt work of freeing managed memory.
 */
static void Z_Free_(gpointer data) {
	z_block_t *z = (z_block_t *) data;

	if (z->children) {
		g_list_free_full(z->children, Z_Free_);
		z->children = NULL;
	}

	if (z->parent) {
		z->parent->children = g_list_remove(z->parent->children, data);
	} else {
		z_state.blocks = g_list_remove(z_state.blocks, data);
	}

	free(z);
}

/*
 * @brief Free an allocation of managed memory.
 */
void Z_Free(void *p) {
	z_block_t *z = ((z_block_t *) p) - 1;

	if (z->magic != Z_MAGIC) {
		Com_Error(ERR_FATAL, "Z_Free: Bad magic for %p.\n", p);
	}

	SDL_mutexP(z_state.lock);

	Z_Free_(z);

	SDL_mutexV(z_state.lock);
}

/*
 * @brief Free all managed items allocated with the specified tag.
 */
void Z_FreeTag(z_tag_t tag) {

	SDL_mutexP(z_state.lock);

	GList *e = z_state.blocks;

	while (e) {
		GList *next = e->next;

		z_block_t *z = (z_block_t *) e->data;

		if (tag == Z_TAG_ALL || z->tag == tag) {
			Z_Free_(z);
		}

		e = next;
	}

	SDL_mutexV(z_state.lock);
}

/*
 * @brief Performs the grunt work of allocating a z_block_t and inserting it
 * into the managed memory structures.
 */
static void *Z_Malloc_(size_t size, z_tag_t tag, z_block_t *parent) {
	const size_t s = size + sizeof(z_block_t);

	z_block_t *z = malloc(s);
	if (!z) {
		Com_Error(ERR_FATAL, "Z_Malloc_: Failed to allocate "Q2W_SIZE_T" bytes.\n", s);
	}

	memset(z, 0, s);

	z->magic = Z_MAGIC;
	z->parent = parent;

	SDL_mutexP(z_state.lock);

	if (z->parent) {
		z->parent->children = g_list_append(z->parent->children, z);
		z->tag = z->parent->tag;
	} else {
		z_state.blocks = g_list_append(z_state.blocks, z);
		z->tag = tag;
	}

	SDL_mutexV(z_state.lock);

	return (void *) (z + 1);
}

/*
 * @brief Allocates a block of managed memory with the specified tag.
 *
 * @param tag Tags allow related objects to be freed in bulk e.g. when a
 * subsystem quits.
 *
 * @return A block of memory initialized to 0x0.
 */
void *Z_TagMalloc(size_t size, z_tag_t tag) {
	return Z_Malloc_(size, tag, NULL);
}

/*
 * @brief Allocates a block of managed memory with the specified parent.
 *
 * @param parent The parent block previously allocated through Z_Malloc /
 * Z_TagMalloc, or NULL. If specified, the returned block will automatically be
 * released when the parent is freed through Z_Free.
 *
 * @return A block of memory initialized to 0x0.
 */
void *Z_LinkMalloc(size_t size, void *parent) {

	if (parent) {
		parent = ((z_block_t *) parent) - 1;
	} else {
		Com_Error(ERR_FATAL, "Z_LinkMalloc: NULL parent\n");
	}

	return Z_Malloc_(size, Z_TAG_DEFAULT, parent);
}

/*
 * @brief Allocates a block of managed memory. All managed memory is freed when
 * the game exits, but may be explicitly freed with Z_Free.
 *
 * @return A block of memory initialized to 0x0.
 */
void *Z_Malloc(size_t size) {
	return Z_Malloc_(size, Z_TAG_DEFAULT, NULL);
}

/*
 * @brief Allocates and returns a copy of the specified string.
 */
char *Z_CopyString(const char *in) {
	char *out;

	out = Z_Malloc(strlen(in) + 1);
	strcpy(out, in);

	return out;
}

/*
 * @brief Initializes the managed memory subsystem. This should be one of the first
 * subsystems initialized by Quake2World.
 */
void Z_Init(void) {

	memset(&z_state, 0, sizeof(z_state));

	z_state.lock = SDL_CreateMutex();
}

/*
 * @brief Shuts down the managed memory subsystem. This should be one of the last
 * subsystems brought down by Quake2World.
 */
void Z_Shutdown(void) {

	g_list_free_full(z_state.blocks, Z_Free_);

	SDL_DestroyMutex(z_state.lock);
}
