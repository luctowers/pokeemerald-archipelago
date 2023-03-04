EventScript_RepelWoreOff::
	lockall
	checkitem VAR_REPEL_LAST_USED, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_RepelWoreOff_NoMoreRepels
	msgbox Text_RepelWoreOff_UseAnother, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_RepelWoreOff_ChooseNo
	copyvar VAR_0x8004, VAR_REPEL_LAST_USED
	specialvar VAR_RESULT, GetItemHoldEffect
	copyvar VAR_REPEL_STEP_COUNT, VAR_RESULT
	bufferitemname 1, VAR_REPEL_LAST_USED
	removeitem VAR_REPEL_LAST_USED, 1
	playse SE_REPEL
	msgbox gText_PlayerUsedVar2, MSGBOX_SIGN
	releaseall
	end

EventScript_RepelWoreOff_ChooseNo::
	closemessage
	releaseall
	end

EventScript_RepelWoreOff_NoMoreRepels::
	msgbox Text_RepelWoreOff, MSGBOX_SIGN
	releaseall
	end

Text_RepelWoreOff:
	.string "REPEL's effect wore off…$"

Text_RepelWoreOff_UseAnother:
	.string "REPEL's effect wore off…\n"
	.string "Use another?$"
