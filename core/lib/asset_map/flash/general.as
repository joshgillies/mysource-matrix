
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

/**
* takes an array and a value and removes the first 
* element in the array with that value
*
* @param Array arr	the array to remove the element from
* @param mixed val	the value to match
*
*/
function array_remove_element(arr, val) {

	var i = array_search(arr, val);
	if (i != null) {
		arr.splice(i, 1);
	}// end if

}// end array_remove_element()

/**
* Takes an array and returns a copy of it
* Useful because arrays get passed by reference to fns
*
* @param Array arr	the array to copy
*
*/
function array_copy(arr) {

	var new_arr = new Array();
	for (var i = 0; i < arr.length; i++) {
		new_arr[i] = arr[i];
	}
	return new_arr;

}// end array_copy()

/**
* Returns an array of all values that are in arr1 but not in arr2
*
* @param Array arr1	
* @param Array arr2	
*
*/
function array_diff(arr1, arr2) {


	var new_arr = new Array();
	for (var i = 0; i < arr1.length; i++) {
		if (array_search(arr2, arr1[i]) == null) {
			new_arr.push(arr1[i]);
		}
	}

	return new_arr;

}// end array_diff()
