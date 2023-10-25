R"(
var azurreChangeRanksManually = function () {

	if (!LobbyAPI.BIsHost()) return;

	var getTarget = PartyListAPI.GetXuidByIndex(playerID);

	azurreUnregister();

	var updateMessageTemp = "";

	var playerData = {
		update: {
			members: {
			}
		}
	};

	if (playerID < 0) {

		for (var id = 0; id < PartyListAPI.GetCount(); id++) {
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/ranking " + rank + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/ranktype " + rankType + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/level " + level + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/xppts " + xp + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/prime " + (prime === 1 || prime === true ? "1" : "0") + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/commends " + `[f${t1}][t${t2}][l${t3}] `;
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/teamcolor " + teamColor + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/loc " + loc + " ";
			updateMessageTemp += "Update/Members/machine" + id + "/player0/game/medals " + '[!' + medal + ']' + " ";
		}
		updateMessage = updateMessageTemp;

		azurreLCS = $.RegisterForUnhandledEvent("PanoramaComponent_Lobby_PlayerUpdated", function (xuid) {
			PartyListAPI.UpdateSessionSettings(updateMessage);
		});

		PartyListAPI.UpdateSessionSettings(updateMessage);
		$.Msg("Changed for all");
		return;
	}

	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/ranking " + rank + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/ranktype " + rankType + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/level " + level + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/xppts " + xp + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/prime " + (prime === 1 || prime === true ? "1" : "0") + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/commends " + `[f${t1}][t${t2}][l${t3}] `;
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/teamcolor " + teamColor + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/loc " + loc + " ";
	updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/medals " + '[!' + medal + ']' + " ";

	switch (playerID) {
		default:
		case 0: machine0 = updateMessageTemp; break;
		case 1: machine1 = updateMessageTemp; break;
		case 2: machine2 = updateMessageTemp; break;
		case 3: machine3 = updateMessageTemp; break;
		case 4: machine4 = updateMessageTemp; break;
		case 5: machine5 = updateMessageTemp; break;
		case 6: machine6 = updateMessageTemp; break;
		case 7: machine7 = updateMessageTemp; break;
		case 8: machine8 = updateMessageTemp; break;
		case 9: machine9 = updateMessageTemp; break;
	}

	if (machine0.length > 1) updateMessage += machine0;
	if (machine1.length > 1) updateMessage += machine1;
	if (machine2.length > 1) updateMessage += machine2;
	if (machine3.length > 1) updateMessage += machine3;
	if (machine4.length > 1) updateMessage += machine4;
	if (machine5.length > 1) updateMessage += machine5;
	if (machine6.length > 1) updateMessage += machine6;
	if (machine7.length > 1) updateMessage += machine7;
	if (machine8.length > 1) updateMessage += machine8;
	if (machine9.length > 1) updateMessage += machine9;

	azurreLCS = $.RegisterForUnhandledEvent("PanoramaComponent_Lobby_PlayerUpdated", function (xuid) {
		PartyListAPI.UpdateSessionSettings(updateMessage);
		$.Msg("==============================");
		$.Msg("Updated Party Session Settings\n\n");
		$.Msg(`${updateMessage}`);
		$.Msg("==============================");
	});

	PartyListAPI.UpdateSessionSettings(updateMessage);
}

azurreChangeRanksManually();

)"