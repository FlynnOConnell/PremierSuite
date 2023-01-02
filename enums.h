#pragma once

// recent as of 2023

enum playlistID
{
	Unknown = -1337,
	Casual = 0,
	Duel = 1,
	Doubles = 2,
	Standard = 3,
	Chaos = 4,
	PrivateMatch = 6,
	Season = 7,
	OfflineSplitscreen = 8,
	Training = 9,
	RankedSoloDuel = 10,
	RankedTeamDoubles = 11,
	RankedStandard = 13,
	SnowDayPromotion = 15,
	Experimental = 16,
	BasketballDoubles = 17,
	Rumble = 18,
	Workshop = 19,
	UGCTrainingEditor = 20,
	UGCTraining = 21,
	Tournament = 22,
	Breakout = 23,
	FaceIt = 26,
	RankedBasketballDoubles = 27,
	RankedRumble = 28,
	RankedBreakout = 29,
	RankedSnowDay = 30,
	HauntedBall = 31,
	BeachBall = 32,
	Rugby = 33,
	AutoTournament = 34,
	RocketLabs = 35,
	RumShot = 37,
	GodBall = 38,
	BoomerBall = 41,
	GodBallDoubles = 43,
	SpecialSnowDay = 44,
	Football = 46,
	Cubic = 47,
	TacticalRumble = 48,
	SpringLoaded = 49,
	SpeedDemon = 50,
	RumbleBM = 52,
	Knockout = 54,
	Thirdwheel = 55,
};

enum Playlist
{
	CASUAL_STANDARD = 0,
	CASUAL_DOUBLES = 1,
	CASUAL_DUELS = 2,
	CASUAL_CHAOS = 3,
	RANKED_STANDARD = 4,
	RANKED_DOUBLES = 5,
	RANKED_DUELS = 6,
	AUTO_TOURNAMENT = 12,
	EXTRAS_RUMBLE = 13,
	EXTRAS_DROPSHOT = 14,
	EXTRAS_HOOPS = 15,
	EXTRAS_SNOWDAY = 16
};

enum PlaylistCategory
{
	CASUAL = 0,
	RANKED = 1,
	EXTRAS = 2
};