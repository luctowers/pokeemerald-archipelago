.include "data/text/archipelago.inc"

ArchipelagoScript_ReceiveRemoteItem::
	specialvar VAR_RESULT, ArchipelagoSpecial_ConsumeReceivedItem
	giveitem VAR_RESULT
	releaseall
	end

ArchipelagoScript_ReceiveRemoteItemSilent::
	specialvar VAR_RESULT, ArchipelagoSpecial_ConsumeReceivedItem
	additem VAR_RESULT, AMOUNT
	end

ArchipelagoScript_FoundArchipelagoItem::
	call EventScript_PlayFanfareObtainedItem
	msgbox gText_PlayerFoundArchipelagoItem, MSGBOX_DEFAULT
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
	special SetHiddenItemFlag
	specialvar VAR_RESULT, ArchipelagoSpecial_IsItemBadge
	goto_if_eq VAR_RESULT, TRUE, ArchipelagoScript_HandleBadgeItem
	call ArchipelagoScript_FoundArchipelagoItem
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
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox RustboroCity_Gym_Text_ReceivedStoneBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE01_GET
	release
	return

ArchipelagoScript_ReceiveKnuckleBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox DewfordTown_Gym_Text_ReceivedKnuckleBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE02_GET
	release
	return

ArchipelagoScript_ReceiveDynamoBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox MauvilleCity_Gym_Text_ReceivedDynamoBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE03_GET
	release
	return

ArchipelagoScript_ReceiveHeatBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox LavaridgeTown_Gym_1F_Text_ReceivedHeatBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE04_GET
	release
	return

ArchipelagoScript_ReceiveBalanceBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox PetalburgCity_Gym_Text_ReceivedBalanceBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE05_GET
	release
	return

ArchipelagoScript_ReceiveFeatherBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox FortreeCity_Gym_Text_ReceivedFeatherBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE06_GET
	release
	return

ArchipelagoScript_ReceiveMindBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox MossdeepCity_Gym_Text_ReceivedMindBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE07_GET
	release
	return

ArchipelagoScript_ReceiveRainBadge::
	call Common_EventScript_PlayGymBadgeFanfare
	msgbox SootopolisCity_Gym_1F_Text_ReceivedRainBadge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_BADGE08_GET
	release
	return
