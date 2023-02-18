.include "data/text/archipelago.inc"

ArchipelagoScript_ReceiveRemoteItem::
	specialvar VAR_RESULT, GetArchipelagoItemId
	giveitem VAR_RESULT
	special SetArchipelagoFlag
	releaseall
	end

ArchipelagoScript_FoundArchipelagoItem::
	call EventScript_PlayFanfareObtainedItem
	message gText_PlayerFoundArchipelagoItem
	waitfanfare
	waitmessage
  return

ArchipelagoScript_ObtainItem::
	call ArchipelagoScript_FoundArchipelagoItem
	setvar VAR_RESULT, TRUE
	return

ArchipelagoScript_FindItem::
	specialvar VAR_RESULT, ArchipelagoSpecial_IsItemBadge
	goto_if_eq VAR_RESULT, TRUE, ArchipelagoScript_ObtainBadge
	removeobject VAR_LAST_TALKED
  call ArchipelagoScript_FoundArchipelagoItem
	release
	return

ArchipelagoScript_FindHiddenItem::
	specialvar VAR_RESULT, ArchipelagoSpecial_IsItemBadge
	goto_if_eq VAR_RESULT, TRUE, ArchipelagoScript_ObtainBadge
  call ArchipelagoScript_FoundArchipelagoItem
	special SetHiddenItemFlag
	releaseall
	end

ArchipelagoScript_ObtainBadge::
  return

ArchipelagoScript_ReceiveStone_Badge::
  return
