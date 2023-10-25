R"(
var _SetBackgroundMovie = function () {
	var videoPlayer = $('#MainMenuMovie');

	videoPlayer.SetAttributeString('data-type', azurreMovie);


	videoPlayer.SetMovie(azurreMovie);
	//videoPlayer.SetSound( 'UIPanorama.BG_' + azurreMovie );
	videoPlayer.Play();
};

_SetBackgroundMovie();
)"