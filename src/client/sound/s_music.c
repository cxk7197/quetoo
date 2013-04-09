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

#include "s_local.h"
#include "client.h"

typedef struct s_music_state_s {
	s_music_t *default_music;
	s_music_t *current_music;
	GList *playlist;
} s_music_state_t;

static s_music_state_t s_music_state;

/*
 * @brief Retain event listener for s_music_t.
 */
static bool S_RetainMusic(s_media_t *self) {
	s_music_t *music = (s_music_t *) self;

	return GlobMatch("track*", music->media.name);
}

/*
 * @brief Free event listener for s_music_t.
 */
static void S_FreeMusic(s_media_t *self) {
	s_music_t *music = (s_music_t *) self;

	if (music->music) {
		Mix_FreeMusic(music->music);
	}
	if (music->rw) {
		SDL_FreeRW(music->rw);
	}
	if (music->buffer) {
		Z_Free(music->buffer);
	}
}

/*
 * @brief Handles the actual loading of .ogg music files.
 */
static bool S_LoadMusicFile(const char *name, void **buffer, SDL_RWops **rw, Mix_Music **music) {
	char path[MAX_QPATH];

	*music = NULL;

	StripExtension(name, path);
	g_snprintf(path, sizeof(path), "music/%s.ogg", name);

	int32_t len;
	if ((len = Fs_LoadFile(path, buffer)) != -1) {

		if ((*rw = SDL_RWFromMem(*buffer, len))) {

			if ((*music = Mix_LoadMUS_RW(*rw))) {
				Com_Debug("S_LoadMusicFile: Loaded %s\n", name);
			} else {
				Com_Warn("S_LoadMusicFile: Failed to load %s: %s\n", name, Mix_GetError());
				SDL_FreeRW(*rw);
			}
		} else {
			Com_Warn("S_LoadMusicFile: Failed to create SDL_RWops for %s\n", name);
			Fs_FreeFile(*buffer);
		}
	} else {
		Com_Debug("S_LoadMusicFile: Failed to load %s\n", name);
	}

	return *music != NULL;
}

/*
 * @brief Clears the musics playlist so that it may be rebuilt.
 */
void S_FlushPlaylist(void) {

	g_list_free(s_music_state.playlist);

	s_music_state.playlist = NULL;
}

/*
 * @brief Loads the music by the specified name.
 */
s_music_t *S_LoadMusic(const char *name) {
	char key[MAX_QPATH];
	s_music_t *music;

	StripExtension(name, key);

	if (!(music = (s_music_t *) S_FindMedia(key))) {

		void *buffer;
		SDL_RWops *rw;
		Mix_Music *mus;
		if (S_LoadMusicFile(key, &buffer, &rw, &mus)) {

			music = (s_music_t *) S_MallocMedia(key, sizeof(s_music_t));

			music->media.Retain = S_RetainMusic;
			music->media.Free = S_FreeMusic;

			music->buffer = buffer;
			music->rw = rw;
			music->music = mus;

			s_music_state.playlist = g_list_append(s_music_state.playlist, music);

			S_RegisterMedia((s_media_t *) music);
		} else {
			Com_Debug("S_LoadMusic: Couldn't load %s\n", key);
			music = NULL;
		}
	}

	return music;
}

/*
 * @brief Stops music playback.
 */
static void S_StopMusic(void) {

	Mix_HaltMusic();

	s_music_state.current_music = NULL;
}

/*
 * @brief Begins playback of the specified s_music_t.
 */
static void S_PlayMusic(s_music_t *music) {

	Mix_PlayMusic(music->music, 1);

	s_music_state.current_music = music;
}

/*
 * @brief Returns the next track in the configured playlist.
 */
static s_music_t *S_NextMusic(void) {
	GList *elt;

	if ((elt = g_list_find(s_music_state.playlist, s_music_state.current_music))) {

		if (elt->next) {
			return (s_music_t *) elt->next->data;
		} else {
			return g_list_nth_data(s_music_state.playlist, 0);
		}
	}

	return s_music_state.default_music;
}

/*
 * @brief Ensures music playback continues by selecting a new track when one
 * completes.
 */
void S_FrameMusic(void) {
	extern cl_static_t cls;
	s_music_t *music;

	if (s_music_volume->modified) {
		s_music_volume->value = Clamp(s_music_volume->value, 0.0, 1.0);

		if (s_music_volume->value)
			Mix_VolumeMusic(s_music_volume->value * 255);
		else
			S_StopMusic();
	}

	if (!s_music_volume->value)
		return;

	music = s_music_state.default_music;

	if (cls.state == CL_ACTIVE) { // try level-specific music

		if (!Mix_PlayingMusic() || (s_music_state.current_music == s_music_state.default_music)) {

			if ((music = S_NextMusic()) != s_music_state.current_music)
				S_StopMusic();
		}
	} else { // select the default music

		if (s_music_state.current_music != s_music_state.default_music)
			S_StopMusic();
	}

	if (!Mix_PlayingMusic()) // play it
		S_PlayMusic(music);
}

/*
 * @brief Plays the next track in the configured playlist.
 */
void S_NextTrack_f(void) {
	S_PlayMusic(S_NextMusic());
}

/*
 * @brief Initializes the music state, loading the default track.
 */
void S_InitMusic(void) {

	memset(&s_music_state, 0, sizeof(s_music_state));

	s_music_state.default_music = S_LoadMusic("track1");
}

/*
 * @brief Shuts down music playback.
 */
void S_ShutdownMusic(void) {
	S_StopMusic();
}
