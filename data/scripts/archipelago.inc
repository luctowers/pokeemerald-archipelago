.include "data/text/archipelago.inc"

ArchipelagoScript_ReceiveRemoteItem::
	specialvar VAR_RESULT, ArchipelagoSpecial_GetReceivedItemId
	giveitem VAR_RESULT
	special ArchipelagoSpecial_SetArchipelagoFlag
	releaseall
	end

ArchipelagoScript_FoundArchipelagoItem::
	call EventScript_PlayFanfareObtainedItem
	message gText_PlayerFoundArchipelagoItem
	waitfanfare
	waitmessage
  return

ArchipelagoScript_ObtainItem::
	specialvar VAR_RESULT, ArchipelagoSpecial_IsItemBadge
	goto_if_eq VAR_RESULT, TRUE, ArchipelagoScript_HandleBadgeItem
	call ArchipelagoScript_FoundArchipelagoItem
	setvar VAR_RESULT, TRUE
	return

ArchipelagoScript_FindItem::
	removeobject VAR_LAST_TALKED
	specialvar VAR_RESULT, ArchipelagoSpecial_IsItemBadge
	goto_if_eq VAR_RESULT, TRUE, ArchipelagoScript_HandleBadgeItem
  call ArchipelagoScript_FoundArchipelagoItem
	release
	return

ArchipelagoScript_FindHiddenItem::
	specialvar VAR_RESULT, ArchipelagoSpecial_IsItemBadge
	goto_if_eq VAR_RESULT, TRUE, ArchipelagoScript_HandleBadgeItem
  call ArchipelagoScript_FoundArchipelagoItem
	special SetHiddenItemFlag
	releaseall
	end

ArchipelagoScript_HandleBadgeItem::
	switch VAR_UNUSED_0x8014
	case ITEM_BADGE_1, ArchipelagoScript_ReceiveStoneBadge
	case ITEM_BADGE_2, ArchipelagoScript_ReceiveKnuckleBadge
	case ITEM_BADGE_3, ArchipelagoScript_ReceiveDynamoBadge
	case ITEM_BADGE_4, ArchipelagoScript_ReceiveHeatBadge
	case ITEM_BADGE_5, ArchipelagoScript_ReceiveBalanceBadge
	case ITEM_BADGE_6, ArchipelagoScript_ReceiveFeatherBadge
	case ITEM_BADGE_7, ArchipelagoScript_ReceiveMindBadge
	case ITEM_BADGE_8, ArchipelagoScript_ReceiveRainBadge
  return

ArchipelagoScript_ReceiveStoneBadge::
	message RustboroCity_Gym_Text_ReceivedStoneBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE01_GET
	release
  return

ArchipelagoScript_ReceiveKnuckleBadge::
	message DewfordTown_Gym_Text_ReceivedKnuckleBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE02_GET
	release
  return

ArchipelagoScript_ReceiveDynamoBadge::
	message MauvilleCity_Gym_Text_ReceivedDynamoBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE03_GET
	release
  return

ArchipelagoScript_ReceiveHeatBadge::
	message LavaridgeTown_Gym_1F_Text_ReceivedHeatBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE04_GET
	release
  return

ArchipelagoScript_ReceiveBalanceBadge::
	message PetalburgCity_Gym_Text_ReceivedBalanceBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE05_GET
	release
  return

ArchipelagoScript_ReceiveFeatherBadge::
	message FortreeCity_Gym_Text_ReceivedFeatherBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE06_GET
	release
  return

ArchipelagoScript_ReceiveMindBadge::
	message MossdeepCity_Gym_Text_ReceivedMindBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE07_GET
	release
  return

ArchipelagoScript_ReceiveRainBadge::
	message SootopolisCity_Gym_1F_Text_ReceivedRainBadge
	waitmessage
	call Common_EventScript_PlayGymBadgeFanfare
	setflag FLAG_BADGE08_GET
	release
  return
