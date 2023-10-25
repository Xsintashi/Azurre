R"(

var lcsWaitPUpdate = null;

var m_gResponseChat = function (msg) {
	msg = msg.split(' ').join('\u{00A0}')
	PartyListAPI.SessionCommand('Game::Chat', 'run all xuid ' + MyPersonaAPI.GetXuid() + ' chat [azurre]\u{00A0}->\u2029' + msg);
};

var m_gFindPlayer = function (str) {
	// Name 
	for (var i = 0; PartyListAPI.GetCount() > i; ++i) {
		var xuid = PartyListAPI.GetXuidByIndex(i);
		var username = PartyListAPI.GetFriendName(xuid);
		if (username.toLowerCase().indexOf(str) != -1)
			return username;
	}
	// Index
	for (var i = 0; PartyListAPI.GetCount() > i; ++i) {
		if (parseInt(str) == i)
			return PartyListAPI.GetFriendName(PartyListAPI.GetXuidByIndex(i));
	}

	return '';
}

var m_gHandleChatCommand = function (username, msg) {
	var args = msg.toLowerCase().split(' ');
	var target = (args.length > 1 ? m_gFindPlayer(args[1]) : username);
	var target_found = (args.length > 1 && target.length > 0);
	if (!target_found) target = username;
	if (args[0] == '!help' || args[0] == '!cmds') {
		var cmds = [
			'!vac <lobbyindex>',
			'!iq <partial:name>|<lobbyindex>',
			'!dick <partial:name>|<lobbyindex>',
			'!gay <partial:name>|<lobbyindex>',
			'!furry <partial:name>|<lobbyindex>',
			'!8ball <question>',
			'!loc <partial:name>',
			'!love <partial:name>',
			'!startMM (start queue)',
			'!stopMM (stop queue)',
			'!getIdxs (prints players index)',
			'!set <idx> <lvl> <xp> <rank> <ranktype>\u2029<f> <t> <l> <prime> <color> <medal> <loc>'
		];

		msg = '\u2029';
		for (var i = 0; cmds.length > i; ++i)
			msg += cmds[i] + '\u2029';

		return m_gResponseChat(msg);
	}

	if (args[0] == '!iq')
		return m_gResponseChat('IQ of ' + target + ' is ' + Math.floor(Math.random() * 420) + '.');
	if (args[0] == '!vac') {
		var playerID = parseInt(args[1]);
		return PartyListAPI.SessionCommand('Game::ChatReportError', `run all xuid ${PartyListAPI.GetXuidByIndex(playerID)} error #SFUI_QMM_ERROR_X_VacBanned`);
	}
	if (args[0] == '!dick')  /// HERE ^^^
		return m_gResponseChat('Dick size of ' + target + ' is ' + Math.floor(Math.random() * 40) + ' cm.');
	if (args[0] == '!gay')
		return m_gResponseChat(target + ' is ' + Math.floor(Math.random() * 101) + '% gay.');
	if (args[0] == '!furry')
		return m_gResponseChat(target + ' is ' + Math.floor(Math.random() * 101) + '% furry.');
	if (args[0] == '!8ball') {
		if (args.length > 1 && args[1].length > 1) {
			var array = [
				'It is certain.', 'It is decidedly so.', 'Without a doubt.', 'Yes definitely.', 'You may rely on it.', 'As I see it, yes.', 'Most likely.', 'Outlook good.', 'Yes.', 'Signs point to yes.',
				'Reply hazy, try again.', 'Ask again later.', 'Better not tell you now.', 'Cannot predict now.', 'Concentrate and ask again.',
				'Don\'t count on it.', 'My reply is no.', 'My sources say no.', 'Not really.', 'My sources say yes.', 'Outlook not so good.', 'Very doubtful.'
			];

			m_gResponseChat('[❽] ' + array[Math.floor(Math.random() * array.length)]);
		} else
			m_gResponseChat('[❽] Maybe ask some question?');
		return;
	}
	if (args[0] == '!loc') {
		msg = '';
		var settings = LobbyAPI.GetSessionSettings();
		for (var i = 0; settings.members.numMachines > i; ++i) {
			var player = settings.members[`machine${i}`];
			if (args.length > 1) {
				if (player.player0.name.toLowerCase().indexOf(target) != -1) {
					msg = player.player0.name + ' is from ' + player.player0.game.loc;
					break;
				}
			}
			else
				msg += player.player0.name + ' is from ' + player.player0.game.loc + '\u2029';
		}
		return m_gResponseChat(msg);
	}
	if (args[0] == '!getidxs') {
		var list = "";
		for (var count = 0; count < PartyListAPI.GetCount(); count++) {
			var playerName = PartyListAPI.GetFriendName(PartyListAPI.GetXuidByIndex(count));
			list += playerName + ':' + count + '\u2029';
		}
		$.Msg(list);
		return m_gResponseChat(list);
	}
	if (args[0] == '!set') {

		if (!LobbyAPI.BIsHost()) return m_gResponseChat("Failed. I'm not the owner of Lobby!");

		var playerID = parseInt(args[1]);
		var getTarget = PartyListAPI.GetXuidByIndex(playerID);

		var level = parseInt(args[2]);
		var xp = parseInt(args[3]);
		var rank = parseInt(args[4]);
		var rankType = parseInt(args[5]);
		var t1 = parseInt(args[6]);
		var t2 = parseInt(args[7]);
		var t3 = parseInt(args[8]);
		var prime = parseInt(args[9]);
		var teamColor = parseInt(args[10]);
		var medal = parseInt(args[11]);
		var loc = args[12];

		azurreUnregister();

		var updateMessageTemp = "";

		var playerData = {
			update: {
				members: {
				}
			}
		}

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
				$.Msg("==============================");
				$.Msg("Updated Party Session Settings\n\n");
				$.Msg(`${updateMessage}`);
				$.Msg("==============================");
			});

			PartyListAPI.UpdateSessionSettings(updateMessage);

			var outputMessage = "";
			outputMessage += "All Players: ";
			outputMessage += "\u2029Level: " + level;
			outputMessage += "\u2029Rank: " + azurreRanks[rank];
			outputMessage += "\u2029Rank Type: " + azurreGetRankType(rankType);
			outputMessage += "\u2029Friendly: " + t1;
			outputMessage += "\u2029Teacher: " + t2;
			outputMessage += "\u2029Leader: " + t3;
			outputMessage += "\u2029Prime: " + prime;
			outputMessage += "\u2029Color: " + azurreGetLobbyColor(teamColor);
			outputMessage += "\u2029Medal: " + medal;
			outputMessage += "\u2029Location: " + loc.toUpperCase();
			outputMessage += "\u2029\u2029Some settings are visible only for other players!";

			return m_gResponseChat(outputMessage);
		}

		updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/ranking " + rank + " ";
		updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/ranktype " + rankType + " ";
		updateMessageTemp += "Update/Members/machine" + playerID + "/player0/game/level " + level + " ";
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
			//$.Msg("==============================");
			//$.Msg("Updated Party Session Settings\n\n");
			//$.Msg(`${updateMessage}`);
			//$.Msg("==============================");
		});

		PartyListAPI.UpdateSessionSettings(updateMessage);

		var outputMessage = "";
		outputMessage += "Player: " + target;
		outputMessage += "\u2029Level: " + level;
		outputMessage += "\u2029Rank: " + azurreRanks[rank];
		outputMessage += "\u2029Rank Type: " + azurreGetRankType(rankType);
		outputMessage += "\u2029Friendly: " + t1;
		outputMessage += "\u2029Teacher: " + t2;
		outputMessage += "\u2029Leader: " + t3;
		outputMessage += "\u2029Prime: " + prime;
		outputMessage += "\u2029Color: " + azurreGetLobbyColor(teamColor);
		outputMessage += "\u2029Medal: " + medal;
		outputMessage += "\u2029Location: " + loc.toUpperCase();
		outputMessage += "\u2029\u2029Some settings are visible only for other players!";

		return m_gResponseChat(outputMessage);
	}
	if (args[0] == '!love') {
		if (target_found)
			m_gResponseChat(username + ' loves ' + target + ' by ' + Math.floor(Math.random() * 101) + '%');
		else
			m_gResponseChat(username + ' love himself by ' + Math.floor(Math.random() * 101) + '%');
		return;
	}

	if (args[0] == '!startMM') return LobbyAPI.StartMatchmaking('', '', '', '');
	if (args[0] == '!stopMM') return LobbyAPI.StopMatchmaking();

}

var m_gHandleChatMessage = function (should_handle) {

	var m_gPartyChat = $.GetContextPanel().FindChildTraverse("PartyChat");
	if (!m_gPartyChat) return;
	var m_gChatLines = m_gPartyChat.FindChildTraverse("ChatLinesContainer") ;
	if (!m_gChatLines) return;

	m_gChatLines.Children().forEach(el => {
		var child = el.GetChild(0)
		if (child && child.BHasClass('left-right-flow') && child.BHasClass('horizontal-align-left')) {
			if (!child.BHasClass('aw_handled')) {
				child.AddClass('aw_handled');
				try {
					var InnerChild = child.GetChild(child.GetChildCount() - 1);
					if (InnerChild && InnerChild.text) {
						var Sender = $.Localize('{s:player_name}', InnerChild);
						var Message = $.Localize('{s:msg}', InnerChild);
						if (should_handle && Message[0] == '!')
							m_gHandleChatCommand(Sender, Message);
					}
				}
				catch (err) { $.Msg(`Error in m_gHandleChatMessage: ${err}`); }
			}
		}
	})
}

var m_gLobbyManager_Commands = true;

var m_gLobbyManager = function () {
	if (GameStateAPI.IsConnectedOrConnectingToServer()) {
		m_gLobbyManagerActive = false;
		return;
	}

	m_gLobbyManagerActive = true;
	m_gHandleChatMessage(m_gLobbyManager_Commands);
	// Players
	var m_gPlayers = '';
	for (var i = 0; PartyListAPI.GetCount() > i; ++i) {
		if (i != 0) m_gPlayers += '᠋';
		m_gPlayers += PartyListAPI.GetFriendName(PartyListAPI.GetXuidByIndex(i));
	}

	$.Schedule(0.1, m_gLobbyManager);
}

if (typeof (m_gLobbyManagerActive) == 'undefined' || m_gLobbyManagerActive == false) m_gLobbyManager();

)"