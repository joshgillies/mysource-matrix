
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


/**
* Takes an array and a value returns the first index
* in the array that matches the passed value, 
* returns null if not found
*
* @param Array arr	the array to search
* @param mixed val	the value to match
*
* @return int
*/
function array_search(arr, val) {

    for (var i = 0; i < arr.length; i++) {
        if (arr[i] == val) return i;
    }
    return null;

}// end array_search()
