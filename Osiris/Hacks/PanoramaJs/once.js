R"(
var azurreLCS = null;
var aLobbyRetarded = false;
var aLobbyRankParty = false;
var machine0 = "";
var machine1 = "";
var machine2 = "";
var machine3 = "";
var machine4 = "";
var machine5 = "";
var machine6 = "";
var machine7 = "";
var machine8 = "";
var machine9 = "";
var updateMessage = "";

function aRandomInt(min, max) { // min and max included 
    return Math.floor(Math.random() * (max - min + 1) + min)
}

var azurreRanks = [
    "Unranked/Expired",
    "Silver I",
    "Silver II",
    "Silver III",
    "Silver IV",
    "Silver Elite",
    "Silver Master Elite",
    "Gold Nova I",
    "Gold Nova II",
    "Gold Nova III",
    "Gold Nova Master",
    "Master Guardian I",
    "Master Guardian II",
    "Master Guardian Elite",
    "Distinguished Master Guardian",
    "Legendary Eagle",
    "Legendary Eagle Master",
    "Supreme Master First Class",
    "Global Elite"
];

var azurreGetRankType = function (index) {

    switch (index) {
        default:
        case 6: return "Competitive";
        case 7:return "Wingman";
        case 7:return "Danger Zone";
    }
};

var azurreGetLobbyColor = function (index) {

    switch (index) {
        default:
        case -1: return "Gray";
        case 0: return "Yellow";
        case 1: return "Purple";
        case 2: return "Green";
        case 3: return "Blue";
        case 4: return "Orange";
    }

};

function azurreUnregister() {
	if (azurreLCS != null) {
		$.UnregisterForUnhandledEvent('PanoramaComponent_Lobby_PlayerUpdated', azurreLCS);
		$.Msg("Unregistered Unhandled Event PlayerUpdated\n");
		azurreLCS = null;
	}
}

function azurreUnhook() {
    azurreUnregister();
    m_gHandleChatCommand = function () { return; };

    $.Msg("Unhooked JsPanorama\n");
}

function azurreRetardLobby() {
    aLobbyRetarded = !aLobbyRetarded;
    var updateMessageTemp = "";
    var num;
    num = aLobbyRetarded ? 2 : 1;
    updateMessageTemp += `Update/Members/numMachines ${num} `;
    updateMessageTemp += `Update/Members/numPlayers ${num} `;
    PartyListAPI.UpdateSessionSettings(updateMessageTemp);
}

function azurreLobbyRankParty() {
    if (!aLobbyRankParty)
        return;
    var updateMessageTemp = "";

    for (var id = 0; id < PartyListAPI.GetCount(); id++) {

        var level = aRandomInt(1, 40);
        var xp = aRandomInt(0, 5000);
        var rankType = aRandomInt(1, 3);
        var rank = aRandomInt(1, 18);
        switch (rankType) {
            case 1:
                rankType = 6; break;
            case 2:
                rankType = 7; break;
            case 3:
                rankType = 10; rank = aRandomInt(1, 15); break;
        }
        var t1 = aRandomInt(1, 9999);
        var t2 = aRandomInt(1, 9999);
        var t3 = aRandomInt(1, 9999);
        var prime = 1;
        var teamColor = aRandomInt(-1, 4);
        var medal = aRandomInt(0, 250);

        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/ranking " + rank + " ";
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/ranktype " + rankType + " ";
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/level " + level + " ";
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/xppts " + xp + " ";
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/prime " + (prime === 1 || prime === true ? "1" : "0") + " ";
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/commends " + `[f${t1}][t${t2}][l${t3}] `;
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/teamcolor " + teamColor + " ";
        updateMessageTemp += "Update/Members/machine" + id + "/player0/game/medals " + '[!' + medalIDs[medal] + ']' + " ";
    }

    PartyListAPI.UpdateSessionSettings(updateMessageTemp);

}

var medalIDs = [
     874, // 5 years
     875, // DreamHack SteelSeries 2013 CS:GO Champion
     876, // DreamHack SteelSeries 2013 CS:GO Finalist
     877, // DreamHack SteelSeries 2013 CS:GO Semifinalist
     878, // DreamHack SteelSeries 2013 CS:GO Quarterfinalist
     879, // EMS One Katowice 2014 CS:GO Champion
     880, // EMS One Katowice 2014 CS:GO Finalist
     881, // EMS One Katowice 2014 CS:GO Semifinalist
     882, // EMS One Katowice 2014 CS:GO Quarterfinalist
     883, // ESL One Cologne 2014 CS:GO Champion
     884, // ESL One Cologne 2014 CS:GO Finalist
     885, // ESL One Cologne 2014 CS:GO Semifinalist
     886, // ESL One Cologne 2014 CS:GO Quarterfinalist
     887, // ESL One Cologne 2014 Pick 'Em Challenge Bronze
     888, // ESL One Cologne 2014 Pick 'Em Challenge Silver
     889, // ESL One Cologne 2014 Pick 'Em Challenge Gold
     890, // DreamHack Winter 2014 CS:GO Champion
     891, // DreamHack Winter 2014 CS:GO Finalist
     892, // DreamHack Winter 2014 CS:GO Semifinalist
     893, // DreamHack Winter 2014 CS:GO Quarterfinalist
     894, // DreamHack Winter 2014 Pick 'Em Challenge Bronze
     895, // DreamHack Winter 2014 Pick 'Em Challenge Silver
     896, // DreamHack Winter 2014 Pick 'Em Challenge Gold
     897, // ESL One Katowice 2015 CS:GO Champion
     898, // ESL One Katowice 2015 CS:GO Finalist
     899, // ESL One Katowice 2015 CS:GO Semifinalist
     900, // ESL One Katowice 2015 CS:GO Quarterfinalist
     901, // ESL One Katowice 2015 Pick 'Em Challenge Bronze
     902, // ESL One Katowice 2015 Pick 'Em Challenge Silver
     903, // ESL One Katowice 2015 Pick 'Em Challenge Gold
     904, // ESL One Cologne 2015 CS:GO Champion
     905, // ESL One Cologne 2015 CS:GO Finalist
     906, // ESL One Cologne 2015 CS:GO Semifinalist
     907, // ESL One Cologne 2015 CS:GO Quarterfinalist
     908, // ESL One Cologne 2015 Pick 'Em Challenge Bronze
     909, // ESL One Cologne 2015 Pick 'Em Challenge Silver
     910, // ESL One Cologne 2015 Pick 'Em Challenge Gold
     911, // DreamHack Cluj, //Napoca 2015 Pick 'Em Challenge Bronze
     912, // DreamHack Cluj, //Napoca 2015 Pick 'Em Challenge Silver
     913, // DreamHack Cluj, //Napoca 2015 Pick 'Em Challenge Gold
     914, // DreamHack Cluj, //Napoca 2015 Fantasy Team Bronze
     915, // DreamHack Cluj, //Napoca 2015 Fantasy Team Silver
     916, // DreamHack Cluj, //Napoca 2015 Fantasy Team Gold
     917, // DreamHack Cluj, //Napoca 2015 CS:GO Champion
     918, // DreamHack Cluj, //Napoca 2015 CS:GO Finalist
     919, // DreamHack Cluj, //Napoca 2015 CS:GO Semifinalist
     920, // DreamHack Cluj, //Napoca 2015 CS:GO Quarterfinalist
     921, // MLG Columbus 2016 Pick 'Em Challenge Bronze
     922, // MLG Columbus 2016 Pick 'Em Challenge Silver
     923, // MLG Columbus 2016 Pick 'Em Challenge Gold
     924, // MLG Columbus 2016 Fantasy Team Bronze
     925, // MLG Columbus 2016 Fantasy Team Silver
     926, // MLG Columbus 2016 Fantasy Team Gold
     927, // MLG Columbus 2016 CS:GO Champion
     928, // MLG Columbus 2016 CS:GO Finalist
     929, // MLG Columbus 2016 CS:GO Semifinalist
     930, // MLG Columbus 2016 CS:GO Quarterfinalist
     931, // ESL One Cologne 2016 CS:GO Champion
     932, // ESL One Cologne 2016 CS:GO Finalist
     933, // ESL One Cologne 2016 CS:GO Semifinalist
     934, // ESL One Cologne 2016 CS:GO Quarterfinalist
     935, // Cologne 2016 Pick 'Em Challenge Bronze
     936, // Cologne 2016 Pick 'Em Challenge Silver
     937, // Cologne 2016 Pick 'Em Challenge Gold
     938, // Cologne 2016 Fantasy Team Challenge Bronze
     939, // Cologne 2016 Fantasy Team Challenge Silver
     940, // Cologne 2016 Fantasy Team Challenge Gold
     941, // ELEAGUE Atlanta 2017 CS:GO Champion
     942, // ELEAGUE Atlanta 2017 CS:GO Finalist
     943, // ELEAGUE Atlanta 2017 CS:GO Semifinalist
     944, // ELEAGUE Atlanta 2017 CS:GO Quarterfinalist
     945, // Atlanta 2017 Pick 'Em Challenge Bronze
     946, // Atlanta 2017 Pick 'Em Challenge Silver
     947, // Atlanta 2017 Pick 'Em Challenge Gold
     948, // PGL Krakow 2017 CS:GO Champion
     949, // PGL Krakow 2017 CS:GO Finalist
     950, // PGL Krakow 2017 CS:GO Semifinalist
     951, // PGL Krakow 2017 CS:GO Quarterfinalist
     952, // Krakow 2017 Pick 'Em Challenge Bronze
     953, // Krakow 2017 Pick 'Em Challenge Silver
     954, // Krakow 2017 Pick 'Em Challenge Gold
     955, // ELEAGUE Boston 2018 CS:GO Champion
     956, // ELEAGUE Boston 2018 CS:GO Finalist
     957, // ELEAGUE Boston 2018 CS:GO Semifinalist
     958, // ELEAGUE Boston 2018 CS:GO Quarterfinalist
     959, // Boston 2018 Pick 'Em Challenge Bronze
     960, // Boston 2018 Pick 'Em Challenge Silver
     961, // Boston 2018 Pick 'Em Challenge Gold
     962, // FACEIT London 2018 CS:GO Champion
     963, // FACEIT London 2018 CS:GO Finalist
     964, // FACEIT London 2018 CS:GO Semifinalist
     965, // FACEIT London 2018 CS:GO Quarterfinalist
     966, // London 2018 Pick 'Em Challenge Bronze
     967, // London 2018 Pick 'Em Challenge Silver
     968, // London 2018 Pick 'Em Challenge Gold
     969, // 10 years
     970, // Loyalty
     971, // IEM Katowice 2019 CS:GO Champion
     972, // IEM Katowice 2019 CS:GO Finalist
     973, // IEM Katowice 2019 CS:GO Semifinalist
     974, // IEM Katowice 2019 CS:GO Quarterfinalist
     975, // StarLadder Berlin 2019 CS:GO Champion
     976, // StarLadder Berlin 2019 CS:GO Finalist
     977, // StarLadder Berlin 2019 CS:GO Semifinalist
     978, // StarLadder Berlin 2019 CS:GO Quarterfinalist
     979, // PGL Stockholm 2021 CS:GO Champion
     980, // PGL Stockholm 2021 CS:GO Finalist
     981, // PGL Stockholm 2021 CS:GO Semifinalist
     982, // PGL Stockholm 2021 CS:GO Quarterfinalist
     983, // PGL Antwerp 2022 CS:GO Champion
     984, // PGL Antwerp 2022 CS:GO Finalist
     985, // PGL Antwerp 2022 CS:GO Semifinalist
     986, // PGL Antwerp 2022 CS:GO Quarterfinalist
     987, // 10 years birthday
	 992, // BLAST.tv Paris 2023 CS:GO Champion
	 993, // BLAST.tv Paris 2023 CS:GO Finalist
	 994, // BLAST.tv Paris 2023 CS:GO Semifinalist
	 995, // BLAST.tv Paris 2023 CS:GO Quarterfinalist
    1001, // Operation Payback #1
    1002, // Operation Payback #2
    1003, // Operation Payback #3
    1013, // Operation Bravo #1
    1014, // Operation Bravo #2
    1015, // Operation Bravo #3
    1024, // Operation Phoenix #1
    1025, // Operation Phoenix #2
    1026, // Operation Phoenix #3
    1028, // Operation Breakout #1
    1029, // Operation Breakout #2
    1030, // Operation Breakout #3
    1316, // Operation Vanguard #1
    1317, // Operation Vanguard #2
    1318, // Operation Vanguard #3
    1327, // Operation Bloodhound #1
    1328, // Operation Bloodhound #2
    1329, // Operation Bloodhound #3
    1331, // 2015 Service #1
    1332, // 2015 Service #2
    1336, // Operation Wildfire #1
    1337, // Operation Wildfire #2
    1338, // Operation Wildfire #3
    1339, // 2016 Service #1
    1340, // 2016 Service #2
    1341, // 2016 Service #3
    1342, // 2016 Service #4
    1343, // 2016 Service #5
    1344, // 2016 Service #6
    1357, // 2017 Service #1
    1358, // 2017 Service #2
    1359, // 2017 Service #3
    1360, // 2017 Service #4
    1361, // 2017 Service #5
    1362, // 2017 Service #6
    1363, // 2017 Service #7
    1367, // 2018 Service #1
    1368, // 2018 Service #2
    1369, // 2018 Service #3
    1370, // 2018 Service #4
    1371, // 2018 Service #5
    1372, // 2018 Service #6
    1376, // 2019 Service #1
    1377, // 2019 Service #2
    1378, // 2019 Service #3
    1379, // 2019 Service #4
    1380, // 2019 Service #5
    1381, // 2019 Service #6
    4353, // Operation Hydra #1
    4354, // Operation Hydra #2
    4355, // Operation Hydra #3
    4356, // Operation Hydra #3
    4550, // Operation Shattered #1
    4551, // Operation Shattered #2
    4552, // Operation Shattered #3
    4553, // Operation Shattered #4
    4674, // 2020 Service #1
    4675, // 2020 Service #2
    4676, // 2020 Service #3
    4677, // 2020 Service #4
    4678, // 2020 Service #5
    4679, // 2020 Service #6
    4682, // alyx_04
    4683, // alyx_07
    4684, // alyx_09
    4685, // alyx_05
    4686, // alyx_12
    4687, // alyx_01
    4688, // alyx_02
    4689, // alyx_03
    4690, // alyx_06
    4691, // alyx_08
    4692, // alyx_11
	4700, // Operation Broken Fang #1
    4701, // Operation Broken Fang #2
    4702, // Operation Broken Fang #3
    4703, // Operation Broken Fang #4
    4737, // 2021 Service #1
    4738, // 2021 Service #2
    4739, // 2021 Service #3
    4740, // 2021 Service #4
    4741, // 2021 Service #5
    4742, // 2021 Service #6
	4759, // Operation Riptide #1
    4760, // Operation Riptide #2
    4761, // Operation Riptide #3
    4762, // Operation Riptide #4
    4819, // 2022 Service #1
    4820, // 2022 Service #2
    4821, // 2022 Service #3
    4822, // 2022 Service #4
    4823, // 2022 Service #5
    4824, // 2022 Service #6
    4873, // 2023 Service #1
    4874, // 2023 Service #2
    4875, // 2023 Service #3
    4876, // 2023 Service #4
    4877, // 2023 Service #5
    4878, // 2023 Service #6
	4884, // BLAST.tv Paris 2023 CS:GO Bronze
	4885, // BLAST.tv Paris 2023 CS:GO Silver
	4886, // BLAST.tv Paris 2023 CS:GO Gold
	4887, // BLAST.tv Paris 2023 CS:GO Crystal
    6012, // Guardian 2
    6013, // Bravo
    6014, // Baggage
    6015, // Phoenix
    6016, // Office
    6017, // Cobblestone
    6018, // Overpass
    6019, // Bloodhound
    6020, // Cache
    6021, // Valeria
    6022, // Chroma
    6023, // Guardian 3
    6024, // Canals
    6025, // Welcome to the Clutch
    6026, // Death Sentence
    6027, // Inferno
    6028, // Hydra
    6029, // Easy Peasy
    6030, // Aces High
    6031, // Hydra
    6032, // Howl
    6033, // Brigadier General
    6034, // Alyx
    6101, // Dust II
    6102, // Guardian Elite
    6103, // Mirage
    6104, // Inferno
    6105, // Italy
    6106, // Victory
    6107, // Militia
    6108, // Nuke
    6109, // Train
    6110, // Guardian
	
          // MAX 250
];

function printf(out) {
    $.Msg(out);
}

)"