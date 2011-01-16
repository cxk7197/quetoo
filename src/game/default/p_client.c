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

#include "g_local.h"
#include "m_player.h"


/*QUAKED info_player_start(1 0 0)(-16 -16 -24)(16 16 32)
The normal starting point for a level.
*/
void G_info_player_start(edict_t *self){
	G_ProjectSpawn(self);
}

/*QUAKED info_player_intermission(1 0 1)(-16 -16 -24)(16 16 32)
Level intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.
'pitch yaw roll'
*/
void G_info_player_intermission(edict_t *self){
	G_ProjectSpawn(self);
}

/*QUAKED info_player_deathmatch(1 0 1)(-16 -16 -24)(16 16 32)
potential spawning position for deathmatch games
*/
void G_info_player_deathmatch(edict_t *self){
	G_ProjectSpawn(self);
}

/*QUAKED info_player_team1(1 0 1)(-16 -16 -24)(16 16 32)
potential spawning position for team games
*/
void G_info_player_team1(edict_t *self){
	G_ProjectSpawn(self);
}

/*QUAKED info_player_team2(1 0 1)(-16 -16 -24)(16 16 32)
potential spawning position for team games
*/
void G_info_player_team2(edict_t *self){
	G_ProjectSpawn(self);
}


/*
 * P_ClientObituary
 *
 * Make a tasteless death announcement.
 */
static void P_ClientObituary(edict_t *self, edict_t *inflictor, edict_t *attacker){
	int ff, mod;
	char *message, *message2;
	g_client_t *killer;

	ff = means_of_death & MOD_FRIENDLY_FIRE;
	mod = means_of_death & ~MOD_FRIENDLY_FIRE;
	message = NULL;
	message2 = "";

	killer = attacker->client ? attacker->client : self->client;

	if(!g_level.warmup && fraglog != NULL){  // write fraglog

		fprintf(fraglog, "\\%s\\%s\\\n", killer->locals.net_name, self->client->locals.net_name);

		fflush(fraglog);
	}

#ifdef HAVE_MYSQL
	if(!g_level.warmup && mysql != NULL){  // insert to db

		snprintf(sql, sizeof(sql), "insert into frag values(null, now(), '%s', '%s', '%s', %d)",
				 g_level.name, killer->locals.sql_name, self->client->locals.sql_name, mod
				);

		sql[sizeof(sql) - 1] = '\0';
		mysql_query(mysql, sql);
	}
#endif

	switch(mod){
		case MOD_SUICIDE:
			message = "sucks at life";
			break;
		case MOD_FALLING:
			message = "challenged gravity";
			break;
		case MOD_CRUSH:
			message = "likes it tight";
			break;
		case MOD_WATER:
			message = "took a drink";
			break;
		case MOD_SLIME:
			message = "got slimed";
			break;
		case MOD_LAVA:
			message = "did a back flip into the lava";
			break;
		case MOD_TRIGGER_HURT:
			message = "sucks at life";
			break;
	}

	if(attacker == self){
		switch(mod){
			case MOD_G_SPLASH:
				message = "went pop";
				break;
			case MOD_R_SPLASH:
				message = "needs glasses";
				break;
			case MOD_L_DISCHARGE:
				message = "took a toaster bath";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				message = "sucks at life";
				break;
		}
	}

	if(message){  // suicide
		gi.BroadcastPrint(PRINT_MEDIUM, "%s %s.\n", self->client->locals.net_name, message);

		if(g_level.warmup)
			return;

		self->client->locals.score--;

		if((g_level.teams || g_level.ctf) && self->client->locals.team)
			self->client->locals.team->score--;

		return;
	}

	self->enemy = attacker;
	if(attacker && attacker->client){
		switch(mod){
			case MOD_SHOTGUN:
				message = "was gunned down by";
				message2 = "'s pea shooter";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was chewed up by";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "was dry-anal-powerslammed by";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_LIGHTNING:
				message = "was tased by";
				message2 = "'s lightning";
				break;
			case MOD_L_DISCHARGE:
				message = "sipped";
				message2 = "'s discharge";
				break;
			case MOD_RAILGUN:
				message = "was poked by";
				message2 = "'s needledick";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			}

		if(message){

			gi.BroadcastPrint(PRINT_MEDIUM, "%s%s %s %s %s\n", (ff ? "^1TEAMKILL^7 " : ""),
					self->client->locals.net_name, message,
					attacker->client->locals.net_name, message2
			);

			if(g_level.warmup)
				return;

			if(ff)
				attacker->client->locals.score--;
			else
				attacker->client->locals.score++;

			if((g_level.teams || g_level.ctf) && attacker->client->locals.team){  // handle team scores too
				if(ff)
					attacker->client->locals.team->score--;
				else
					attacker->client->locals.team->score++;
			}
		}
	}
}


/*
 * P_TossWeapon
 */
static void P_TossWeapon(edict_t *self){
	g_item_t *item;

	// don't drop weapon when falling into void
	if(means_of_death == MOD_TRIGGER_HURT)
		return;

	item = self->client->locals.weapon;

	if(!self->client->locals.inventory[self->client->ammo_index])
		return;  // don't drop when out of ammo

	G_DropItem(self, item);
}


/*
 * P_TossQuadDamage
 */
void P_TossQuadDamage(edict_t *self){
	edict_t *quad;

	if(!self->client->locals.inventory[quad_damage_index])
		return;

	quad = G_DropItem(self, G_FindItemByClassname("item_quad"));

	if(quad)
		quad->timestamp = self->client->quad_damage_time;

	self->client->quad_damage_time = 0.0;
	self->client->locals.inventory[quad_damage_index] = 0;
}


/*
 * P_TossFlag
 */
void P_TossFlag(edict_t *self){
	g_team_t *ot;
	edict_t *of;
	int index;

	if(!(ot = G_OtherTeam(self->client->locals.team)))
		return;

	if(!(of = G_FlagForTeam(ot)))
		return;

	index = ITEM_INDEX(of->item);

	if(!self->client->locals.inventory[index])
		return;

	self->client->locals.inventory[index] = 0;

	self->s.model_index3 = 0;
	self->s.effects &= ~(EF_CTF_RED | EF_CTF_BLUE);

	gi.BroadcastPrint(PRINT_HIGH, "%s dropped the %s flag\n",
			self->client->locals.net_name, ot->name);

	G_DropItem(self, of->item);
}


/*
 * P_Pain
 */
void P_Pain(edict_t *self, edict_t *other, int damage, int knockback){

	if(other && other->client && other != self){  // play a hit sound
		gi.Sound(other, gi.SoundIndex("misc/hit"), ATTN_STATIC);
	}
}


/*
 * P_Die
 */
void P_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point){

	gi.Sound(self, gi.SoundIndex("*death_1"), ATTN_NORM);

	self->client->respawn_time = g_level.time + 1.0;
	self->client->ps.pmove.pm_type = PM_DEAD;

	P_ClientObituary(self, inflictor, attacker);

	if(!g_level.gameplay && !g_level.warmup)  // drop weapon
		P_TossWeapon(self);

	self->client->new_weapon = NULL;  // reset weapon state
	P_ChangeWeapon(self);

	if(!g_level.gameplay && !g_level.warmup)  // drop quad
		P_TossQuadDamage(self);

	if(g_level.ctf && !g_level.warmup)  // drop flag in ctf
		P_TossFlag(self);

	G_Score_f(self);  // show scores

	self->sv_flags |= SVF_NOCLIENT;

	self->dead = true;
	self->class_name = "dead";

	self->s.model_index = 0;
	self->s.model_index2 = 0;
	self->s.model_index3 = 0;
	self->s.model_index4 = 0;
	self->s.sound = 0;
	self->s.event = 0;
	self->s.effects = 0;

	self->solid = SOLID_NOT;
	self->takedamage = false;

	gi.LinkEntity(self);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_GIB);
	gi.WritePosition(self->s.origin);
	gi.Multicast(self->s.origin, MULTICAST_PVS);
}


/*
 *  Stocks client's inventory with specified item.  Weapons receive
 *  specified quantity of ammo, while health and armor are set to
 *  the specified quantity.
 */
static void P_Give(g_client_t *client, char *it, int quantity){
	g_item_t *item;
	int index;

	if(!strcasecmp(it, "Health")){
		client->locals.health = quantity;
		return;
	}

	if(!strcasecmp(it, "Armor")){
		client->locals.armor = quantity;
		return;
	}

	item = G_FindItem(it);

	if(!item)
		return;

	index = ITEM_INDEX(item);

	if(item->flags & IT_WEAPON){  // weapons receive quantity as ammo
		client->locals.inventory[index] = 1;

		item = G_FindItem(item->ammo);
		index = ITEM_INDEX(item);

		if(quantity > -1)
			client->locals.inventory[index] = quantity;
		else
			client->locals.inventory[index] = item->quantity;
	}
	else {  // while other items receive quantity directly
		if(quantity > -1)
			client->locals.inventory[index] = quantity;
		else
			client->locals.inventory[index] = item->quantity;
	}
}


/*
 * P_GiveLevelLocals
 */
static qboolean P_GiveLevelLocals(g_client_t *client){
	char buf[512], *it, *q;
	int quantity;

	if(*g_level.give == '\0')
		return false;

	strncpy(buf, g_level.give, sizeof(buf));

	it = strtok(buf, ",");

	while(true){

		if(!it)
			break;

		it = Com_TrimString(it);

		if(*it != '\0'){

			if((q = strrchr(it, ' '))){
				quantity = atoi(q + 1);

				if(quantity > -1)  // valid quantity
					*q = 0;
			}
			else
				quantity = -1;

			P_Give(client, it, quantity);
		}

		it = strtok(NULL, ",");
	}

	return true;
}

/*
 * P_InitClientLocals
 */
static void P_InitClientLocals(g_client_t *client){
	g_item_t *item;
	int i;

	// clear inventory
	for(i = 0; i < MAX_ITEMS; i++)
		client->locals.inventory[i] = 0;

	// set max inventory levels
	client->locals.health = 100;
	client->locals.max_health = 100;

	client->locals.armor = 0;
	client->locals.max_armor = 200;

	client->locals.max_shells = 80;
	client->locals.max_bullets = 200;
	client->locals.max_grenades = 50;
	client->locals.max_rockets = 50;
	client->locals.max_cells = 200;
	client->locals.max_bolts = 100;
	client->locals.max_slugs = 50;
	client->locals.max_nukes = 10;

	// instagib gets railgun and slugs, both in normal mode and warmup
	if(g_level.gameplay == INSTAGIB){
		P_Give(client, "Railgun", 1000);
		item = G_FindItem("Railgun");
	}
	// arena or dm warmup yields all weapons, health, etc..
	else if((g_level.gameplay == ARENA) || g_level.warmup){
		P_Give(client, "Railgun", 50);
		P_Give(client, "Lightning", 200);
		P_Give(client, "Hyperblaster", 200);
		P_Give(client, "Rocket Launcher", 50);
		P_Give(client, "Grenade Launcher", 50);
		P_Give(client, "Machinegun", 200);
		P_Give(client, "Super Shotgun", 80);
		P_Give(client, "Shotgun", 80);

		P_Give(client, "Armor", 200);

		item = G_FindItem("Rocket Launcher");
	}
	// dm gets shotgun and 10 shots
	else {
		P_Give(client, "Shotgun", 10);
		item = G_FindItem("Shotgun");
	}

	if(P_GiveLevelLocals(client)){  // use the best weapon we were given by level
		P_NoAmmoWeaponChange(client);
		client->locals.weapon = client->new_weapon;
	}
	else  // or use best given by gameplay
		client->locals.weapon = item;

	// clean up weapon state
	client->locals.lastweapon = NULL;
	client->new_weapon = NULL;
	client->locals.weapon_frame = 0;
}


/*
 * P_EnemyRangeFromSpot
 *
 * Returns the distance to the nearest enemy from the given spot
 */
static float P_EnemyRangeFromSpot(edict_t *ent, edict_t *spot){
	edict_t *player;
	float dist, bestdist;
	vec3_t v;
	int n;

	bestdist = 9999999.0;

	for(n = 1; n <= sv_maxclients->value; n++){
		player = &g_game.edicts[n];

		if(!player->in_use)
			continue;

		if(player->health <= 0)
			continue;

		if(player->client->locals.spectator)
			continue;

		VectorSubtract(spot->s.origin, player->s.origin, v);
		dist = VectorLength(v);

		if(g_level.teams || g_level.ctf){  // avoid collision with team mates

			if(player->client->locals.team == ent->client->locals.team){
				if(dist > 64.0)  // if they're far away, ignore them
					continue;
			}
		}

		if(dist < bestdist)
			bestdist = dist;
	}

	return bestdist;
}


/*
 * P_SelectRandomDeathmatchSpawnPoint
 */
static edict_t *P_SelectRandomSpawnPoint(edict_t *ent, const char *class_name){
	edict_t *spot;
	int count = 0;

	spot = NULL;

	while((spot = G_Find(spot, FOFS(class_name), class_name)) != NULL)
		count++;

	if(!count)
		return NULL;

	count = rand() % count;

	while(count-- >= 0)
		spot = G_Find(spot, FOFS(class_name), class_name);

	return spot;
}


/*
 * P_SelectFarthestDeathmatchSpawnPoint
 */
static edict_t *P_SelectFarthestSpawnPoint(edict_t *ent, const char *class_name){
	edict_t *spot, *bestspot;
	float dist, bestdist;

	spot = bestspot = NULL;
	bestdist = 0.0;

	while((spot = G_Find(spot, FOFS(class_name), class_name)) != NULL){

		dist = P_EnemyRangeFromSpot(ent, spot);

		if(dist > bestdist){
			bestspot = spot;
			bestdist = dist;
		}
	}

	if(bestspot)
		return bestspot;

	// if there is an enemy just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find(NULL, FOFS(class_name), class_name);

	return spot;
}


/*
 * P_SelectDeathmatchSpawnPoint
 */
static edict_t *P_SelectDeathmatchSpawnPoint(edict_t *ent){

	if(g_spawnfarthest->value)
		return P_SelectFarthestSpawnPoint(ent, "info_player_deathmatch");

	return P_SelectRandomSpawnPoint(ent, "info_player_deathmatch");
}


/*
 * P_SelectCaptureSpawnPoint
 */
static edict_t *P_SelectCaptureSpawnPoint(edict_t *ent){
	char *c;

	if(!ent->client->locals.team)
		return NULL;

	c = ent->client->locals.team == &good ?
		"info_player_team1" : "info_player_team2";

	if(g_spawnfarthest->value)
		return P_SelectFarthestSpawnPoint(ent, c);

	return P_SelectRandomSpawnPoint(ent, c);
}


/*
 * P_SelectSpawnPoint
 *
 * Chooses a player start, deathmatch start, etc
 */
static void P_SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles){
	edict_t *spot = NULL;

	if(g_level.teams || g_level.ctf)  // try teams/ctf spawns first if applicable
		spot = P_SelectCaptureSpawnPoint(ent);

	if(!spot)  // fall back on dm spawns (e.g ctf games on dm maps)
		spot = P_SelectDeathmatchSpawnPoint(ent);

	// and lastly fall back on single player start
	if(!spot){
		while((spot = G_Find(spot, FOFS(class_name), "info_player_start")) != NULL){
			if(!spot->target_name)  // hopefully without a target
				break;
		}

		if(!spot){  // last resort, find any
			if((spot = G_Find(spot, FOFS(class_name), "info_player_start")) == NULL)
				gi.Error("P_SelectSpawnPoint: Couldn't find spawn point.");
		}
	}

	VectorCopy(spot->s.origin, origin);
	origin[2] += 12;
	VectorCopy(spot->s.angles, angles);
}


/*
 * P_PutClientInServer
 *
 * The grunt work of putting the client into the server on [re]spawn.
 */
static void P_PutClientInServer(edict_t *ent){
	vec3_t spawn_origin, spawn_angles, old_angles;
	float height;
	g_client_t *client;
	g_client_locals_t locals;
	int i;

	// find a spawn point
	P_SelectSpawnPoint(ent, spawn_origin, spawn_angles);

	client = ent->client;

	// retain last angles for delta
	VectorCopy(ent->client->cmd_angles, old_angles);

	// reset inventory, health, etc
	P_InitClientLocals(client);

	// clear everything but locals
	locals = client->locals;
	memset(client, 0, sizeof(*client));
	client->locals = locals;

	// clear entity values
	VectorScale(PM_MINS, PM_SCALE, ent->mins);
	VectorScale(PM_MAXS, PM_SCALE, ent->maxs);
	height = ent->maxs[2] - ent->mins[2];

	ent->ground_entity = NULL;
	ent->takedamage = true;
	ent->move_type = MOVE_TYPE_WALK;
	ent->view_height = ent->mins[2] + (height * 0.75);
	ent->in_use = true;
	ent->class_name = "player";
	ent->mass = 200.0;
	ent->solid = SOLID_BBOX;
	ent->dead = false;
	ent->jump_time = 0.0;
	ent->gasp_time = 0.0;
	ent->drown_time = g_level.time + 12.0;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/ichabod/tris.md2";
	ent->pain = P_Pain;
	ent->die = P_Die;
	ent->water_level = 0;
	ent->water_type = 0;
	ent->sv_flags = 0;

	// copy these back once they have been set in locals
	ent->health = ent->client->locals.health;
	ent->max_health = ent->client->locals.max_health;

	VectorClear(ent->velocity);
	ent->velocity[2] = 150.0;

	// clear playerstate values
	memset(&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0] * 8;
	client->ps.pmove.origin[1] = spawn_origin[1] * 8;
	client->ps.pmove.origin[2] = spawn_origin[2] * 8;

	// clear entity state values
	ent->s.effects = 0;
	ent->s.model_index = 255;  // will use the skin specified model
	ent->s.model_index2 = 255;  // custom gun model
	// skin_num is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skin_num = ent - g_game.edicts - 1;
	ent->s.model_index3 = 0;
	ent->s.model_index4 = 0;

	ent->s.frame = 0;
	VectorCopy(spawn_origin, ent->s.origin);
	VectorCopy(ent->s.origin, ent->s.old_origin);

	// set the delta angle of the spawn point
	for(i = 0; i < 3; i++){
		client->ps.pmove.delta_angles[i] =
			ANGLE2SHORT(spawn_angles[i] - old_angles[i]);
	}

	VectorClear(client->cmd_angles);
	VectorClear(client->angles);
	VectorClear(ent->s.angles);

	// spawn a spectator
	if(client->locals.spectator){
		client->chase_target = NULL;

		client->locals.weapon = NULL;
		client->locals.team = NULL;
		client->locals.ready = false;

		ent->move_type = MOVE_TYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->sv_flags |= SVF_NOCLIENT;
		ent->takedamage = false;

		gi.LinkEntity(ent);
		return;
	}

	// or spawn a player
	ent->s.event = EV_TELEPORT;

	// hold in place briefly
	client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	client->ps.pmove.pm_time = 20;

	client->locals.match_num = g_level.match_num;
	client->locals.round_num = g_level.round_num;

	gi.UnlinkEntity(ent);

	G_KillBox(ent);  // telefrag anyone in our spot

	gi.LinkEntity(ent);

	// force the current weapon up
	client->new_weapon = client->locals.weapon;
	P_ChangeWeapon(ent);
}


/*
 * P_Respawn
 *
 * In this case, voluntary means that the client has explicitly requested
 * a respawn by changing their spectator status.
 */
void P_Respawn(edict_t *ent, qboolean voluntary){

	P_PutClientInServer(ent);

	// clear scores and match/round on voluntary changes
	if(ent->client->locals.spectator && voluntary){
		ent->client->locals.score = ent->client->locals.captures = 0;
		ent->client->locals.match_num = ent->client->locals.round_num = 0;
	}

	ent->client->respawn_time = g_level.time;

	if(!voluntary)  // dont announce involuntary spectator changes
		return;

	if(ent->client->locals.spectator)
		gi.BroadcastPrint(PRINT_HIGH, "%s likes to watch\n", ent->client->locals.net_name);
	else if(ent->client->locals.team)
		gi.BroadcastPrint(PRINT_HIGH, "%s has joined %s\n", ent->client->locals.net_name,
				ent->client->locals.team->name);
	else
		gi.BroadcastPrint(PRINT_HIGH, "%s wants some\n", ent->client->locals.net_name);
}


/*
 * P_Begin
 *
 * Called when a client has finished connecting, and is ready
 * to be placed into the game.  This will happen every level load.
 */
void P_Begin(edict_t *ent){
	char welcome[256];

	int player_num = ent - g_game.edicts - 1;

	ent->client = g_game.clients + player_num;

	G_InitEdict(ent);

	P_InitClientLocals(ent->client);

	VectorClear(ent->client->cmd_angles);
	ent->client->locals.first_frame = g_level.frame_num;

	// force spectator if match or rounds
	if(g_level.match || g_level.rounds)
		ent->client->locals.spectator = true;
	else if(g_level.teams || g_level.ctf){
		if(g_autojoin->value)
			G_AddClientToTeam(ent, G_SmallestTeam()->name);
		else
			ent->client->locals.spectator = true;
	}

	// spawn them in
	P_Respawn(ent, true);

	if(g_level.intermission_time){
		P_MoveToIntermission(ent);
	} else {
		memset(welcome, 0, sizeof(welcome));

		snprintf(welcome, sizeof(welcome),
				"^2Welcome to ^7http://quake2world.net\n"
				"^2Gameplay is ^1%s\n", G_GameplayName(g_level.gameplay));

		if(g_level.teams)
			strncat(welcome, "^2Teams are enabled\n", sizeof(welcome));

		if(g_level.ctf)
			strncat(welcome, "^2CTF is enabled\n", sizeof(welcome));

		if(g_voting->value)
			strncat(welcome, "^2Voting is allowed\n", sizeof(welcome));

		gi.ClientCenterPrint(ent, "%s", welcome);
	}

	// make sure all view stuff is valid
	P_EndServerFrame(ent);

	srand(time(NULL));  // set random seed
}


/*
 * P_UserInfoChanged
 */
void P_UserInfoChanged(edict_t *ent, const char *user_info){
	const char *s;
	char *c;
	char name[MAX_NET_NAME];
	int player_num, i;
	qboolean color;
	g_client_t *cl;

	// check for malformed or illegal info strings
	if(!Info_Validate(user_info)){
		user_info = "\\name\\newbie\\skin\\ichabod/ichabod";
	}

	cl = ent->client;

	// set name, use a temp buffer to compute length and crutch up bad names
	s = Info_ValueForKey(user_info, "name");

	strncpy(name, s, sizeof(name) - 1);
	name[sizeof(name) - 1] = 0;

	color = false;
	c = name;
	i = 0;

	// trim to 15 printable chars
	while(i < 15){

		if(!*c)
			break;

		if(IS_COLOR(c)){
			color = true;
			c += 2;
			continue;
		}

		c++;
		i++;
	}
	name[c - name] = 0;

	if(!i)  // name had nothing printable
		strcpy(name, "newbie");

	if(color)  // reset to white
		strcat(name, "^7");

	if(strncmp(cl->locals.net_name, name, sizeof(cl->locals.net_name))){

		if(*cl->locals.net_name != '\0')
			gi.BroadcastPrint(PRINT_MEDIUM, "%s changed name to %s\n", cl->locals.net_name, name);

		strncpy(cl->locals.net_name, name, sizeof(cl->locals.net_name) - 1);
		cl->locals.net_name[sizeof(cl->locals.net_name) - 1] = 0;
	}

#ifdef HAVE_MYSQL
	if(mysql != NULL){  // escape name for safe db insertions

		Com_StripColor(cl->locals.net_name, name);

		mysql_real_escape_string(mysql, name, cl->locals.sql_name,
				sizeof(cl->locals.sql_name));
	}
#endif

	// set skin
	if((g_level.teams || g_level.ctf) && cl->locals.team)  // players must use teamskin to change
		s = cl->locals.team->skin;
	else
		s = Info_ValueForKey(user_info, "skin");

	if(*s != '\0')  // something valid-ish was provided
		strncpy(cl->locals.skin, s, sizeof(cl->locals.skin) - 1);
	else {
		strcpy(cl->locals.skin, "ichabod");
		cl->locals.skin[sizeof(cl->locals.skin) - 1] = 0;
	}

	s = cl->locals.skin;

	c = strchr(s, '/');

	// let players use just the model name, client will find skin
	if(!c || *c == '\0'){
		if(c)  // null terminate for strcat
			*c = 0;

		strncat(cl->locals.skin, "/default", sizeof(cl->locals.skin) - 1 - strlen(s));
	}

	// set color
	s = Info_ValueForKey(user_info, "color");
	cl->locals.color = ColorByName(s, 243);

	player_num = ent - g_game.edicts - 1;

	// combine name and skin into a config_string
	gi.ConfigString(CS_PLAYER_SKINS + player_num, va("%s\\%s", cl->locals.net_name, cl->locals.skin));

	// save off the user_info in case we want to check something later
	strncpy(ent->client->locals.user_info, user_info, sizeof(ent->client->locals.user_info) - 1);
}


/*
 * P_Connect
 *
 * Called when a player begins connecting to the server.
 * The game can refuse entrance to a client by returning false.
 * If the client is allowed, the connection process will continue
 * and eventually get to P_Begin()
 * Changing levels will NOT cause this to be called again.
 */
qboolean P_Connect(edict_t *ent, char *user_info){

	// check password
	const char *value = Info_ValueForKey(user_info, "password");
	if(*password->string && strcmp(password->string, "none") &&
			strcmp(password->string, value)){
		Info_SetValueForKey(user_info, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// they can connect
	ent->client = g_game.clients + (ent - g_game.edicts - 1);

	// clean up locals things which are not reset on spawns
	ent->client->locals.score = 0;
	ent->client->locals.team = NULL;
	ent->client->locals.vote = VOTE_NOOP;
	ent->client->locals.spectator = false;
	ent->client->locals.net_name[0] = 0;

	// set name, skin, etc..
	P_UserInfoChanged(ent, user_info);

	if(sv_maxclients->value > 1)
		gi.BroadcastPrint(PRINT_HIGH, "%s connected\n", ent->client->locals.net_name);

	ent->sv_flags = 0; // make sure we start with known default
	return true;
}


/*
 * P_Disconnect
 *
 * Called when a player drops from the server.  Not be called between levels.
 */
void P_Disconnect(edict_t *ent){
	int player_num;

	if(!ent->client)
		return;

	P_TossQuadDamage(ent);
	P_TossFlag(ent);

	gi.BroadcastPrint(PRINT_HIGH, "%s bitched out\n", ent->client->locals.net_name);

	// send effect
	gi.WriteByte(svc_muzzle_flash);
	gi.WriteShort(ent - g_game.edicts);
	gi.WriteByte(MZ_LOGOUT);
	gi.Multicast(ent->s.origin, MULTICAST_PVS);

	gi.UnlinkEntity(ent);

	ent->client->locals.user_info[0] = 0;

	ent->in_use = false;
	ent->solid = SOLID_NOT;
	ent->s.model_index = 0;
	ent->s.model_index2 = 0;
	ent->s.model_index3 = 0;
	ent->s.model_index4 = 0;
	ent->class_name = "disconnected";

	player_num = ent - g_game.edicts - 1;
	gi.ConfigString(CS_PLAYER_SKINS + player_num, "");
}


static edict_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
static trace_t P_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end){
	if(pm_passent->health > 0)
		return gi.Trace(start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.Trace(start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}


/*
 * P_InventoryThink
 */
static void P_InventoryThink(edict_t *ent){

	if(ent->client->locals.inventory[quad_damage_index]){  // if they have quad

		if(ent->client->quad_damage_time < g_level.time){  // expire it

			ent->client->quad_damage_time = 0.0;
			ent->client->locals.inventory[quad_damage_index] = 0;

			gi.Sound(ent, gi.SoundIndex("quad/expire"), ATTN_NORM);

			ent->s.effects &= ~EF_QUAD;
		}
	}

	// other runes and things can be timed out here as well
}


/*
 * P_Think
 *
 * This will be called once for each client frame, which will
 * usually be a couple times for each server frame.
 */
void P_Think(edict_t *ent, user_cmd_t *ucmd){
	g_client_t *client;
	edict_t *other;
	int i, j;
	pm_move_t pm;

	g_level.current_entity = ent;
	client = ent->client;

	if(g_level.intermission_time){
		client->ps.pmove.pm_type = PM_FREEZE;
		return;
	}

	pm_passent = ent;  // ignore ourselves on traces

	if(client->chase_target){  // ensure chase is valid

		client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

		if(!client->chase_target->in_use ||
				client->chase_target->client->locals.spectator){

			other = client->chase_target;

			P_ChaseNext(ent);

			if(client->chase_target == other){  // no one to chase
				client->chase_target = NULL;
			}
		}
	}

	if(!client->chase_target){  // set up for pmove

		client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

		if(ent->move_type == MOVE_TYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if(ent->s.model_index != 255 || ent->dead)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;

        client->ps.pmove.gravity = g_level.gravity;

		memset(&pm, 0, sizeof(pm));
		pm.s = client->ps.pmove;

		for(i = 0; i < 3; i++){
			pm.s.origin[i] = ent->s.origin[i] * 8.0;
			pm.s.velocity[i] = ent->velocity[i] * 8.0;
		}

		pm.cmd = *ucmd;

		pm.trace = P_Trace;  // adds default params
		pm.pointcontents = gi.PointContents;

		// perform a pmove
		gi.Pmove(&pm);

		// save results of pmove
		client->ps.pmove = pm.s;

		for(i = 0; i < 3; i++){
			ent->s.origin[i] = pm.s.origin[i] * 0.125;
			ent->velocity[i] = pm.s.velocity[i] * 0.125;
		}

		VectorCopy(pm.mins, ent->mins);
		VectorCopy(pm.maxs, ent->maxs);

		client->cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		// check for jump, play randomized sound
		if(ent->ground_entity && !pm.ground_entity &&
				(pm.cmd.up >= 10) && (pm.water_level == 0) &&
				ent->jump_time < g_level.time - 0.2){

			if(crand() > 0)
				gi.Sound(ent, gi.SoundIndex("*jump_1"), ATTN_NORM);
			else
				gi.Sound(ent, gi.SoundIndex("*jump_2"), ATTN_NORM);

			ent->jump_time = g_level.time;
		}

		ent->view_height = pm.view_height;
		ent->water_level = pm.water_level;
		ent->water_type = pm.water_type;

		ent->ground_entity = pm.ground_entity;
		if(pm.ground_entity)
			ent->ground_entity_link_count = pm.ground_entity->link_count;

		VectorCopy(pm.angles, client->angles);
		VectorCopy(pm.angles, client->ps.angles);

		gi.LinkEntity(ent);

		// touch jump pads, hurt brushes, etc..
		if(ent->move_type != MOVE_TYPE_NOCLIP && ent->health > 0)
			G_TouchTriggers(ent);

		// touch other objects
		for(i = 0; i < pm.num_touch; i++){

			other = pm.touch_ents[i];

			for(j = 0; j < i; j++)
				if(pm.touch_ents[j] == other)
					break;

			if(j != i)
				continue;  // duplicated

			if(!other->touch)
				continue;

			other->touch(other, ent, NULL, NULL);
		}
	}

	client->old_buttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->old_buttons;

	// fire weapon if requested
	if(client->latched_buttons & BUTTON_ATTACK){
		if(client->locals.spectator){

			client->latched_buttons = 0;

			if(client->chase_target){  // toggle chasecam
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else {
				P_GetChaseTarget(ent);
			}
		} else if(client->weapon_think_time < g_level.time){
			P_WeaponThink(ent);
		}
	}

	// update chase cam if being followed
	for(i = 1; i <= sv_maxclients->value; i++){
		other = g_game.edicts + i;
		if(other->in_use && other->client->chase_target == ent)
			P_UpdateChaseCam(other);
	}

	P_InventoryThink(ent);
}


/*
 * P_BeginServerFrame
 *
 * This will be called once for each server frame, before running
 * any other entities in the world.
 */
void P_BeginServerFrame(edict_t *ent){
	g_client_t *client;

	if(g_level.intermission_time)
		return;

	client = ent->client;

	if(ent->ground_entity)  // let this be reset each frame as needed
		client->ps.pmove.pm_flags &= ~PMF_PUSHED;

	// run weapon think if it hasn't been done by a command
	if(client->weapon_think_time < g_level.time && !client->locals.spectator)
		P_WeaponThink(ent);

	if(ent->dead){  // check for respawn conditions

		// rounds mode implies last-man-standing, force to spectator immediately if round underway
		if(g_level.rounds && g_level.round_time && g_level.time >= g_level.round_time){
			client->locals.spectator = true;
			P_Respawn(ent, false);
		}
		else if(g_level.time > client->respawn_time && client->latched_buttons & BUTTON_ATTACK){
			P_Respawn(ent, false);  // all other respawns require a click from the player
		}
	}

	client->latched_buttons = 0;
}
