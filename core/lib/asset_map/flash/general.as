
_root.pop_up   = false;
_root.progress = false;
_root.loading_xml = false;

function showProgressBar(text) 
{
	if (_root.pop_up) return false;
	_root.pop_up = true;

	_root.progress_bar._visible = true;
	_root.progress_bar.progress_text = text;
	_root.progress_bar.gotoAndPlay(1);

	return true;
}

function hideProgressBar()
{
	_root.progress_bar.stop();
	_root.progress_bar._visible = false;
	_root.pop_up = false;
}

function showDialog(heading, str) 
{
	if (_root.pop_up) return false;
	_root.pop_up = true;

	_root.dialog_box.dialog_heading = heading;
	_root.dialog_box.dialog_text = str;
	_root.dialog_box._visible = true;

	return true;
}

function hideDialog() 
{

	_root.dialog_box._visible = false;
	_root.pop_up = false;

}
