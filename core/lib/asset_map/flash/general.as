
_root.dialog = false;
_root.loading_xml = false;

function showDialog(heading, str) 
{
	if (_root.dialog) return;

	_root.dialog_box.dialog_heading = heading;
	_root.dialog_box.dialog_text = str;
	_root.dialog_box._visible = true;
	_root.dialog = true;

}

function hideDialog() 
{

	_root.dialog_box._visible = false;
	_root.dialog = false;

}


function getAssetKids(parent_assetid) 
{
	if (_root.dialog) return false;
	if (loading_xml)  return false;

	_root.progress_bar._visible = true;
	_root.progress_bar.progress_text = "Loading...";
	_root.progress_bar.gotoAndPlay(1);


	// Load album XML data
	loading_xml = true;
	_root.assetXML.load(asset_xml_load_path + parent_assetid);

	return true;

}

/**
* The onLoad fn for the assetXML object defined in main.as
* Hence why we can use the 'this.' syntax to refer to the XML object
*
*/
function assetXMLonLoad(success) 
{

	loading_xml = false;
	_root.progress_bar.stop();
	_root.progress_bar._visible = false;

	var assetsNode = this.firstChild;

	// something buggered up with the connection
	if (!success || assetsNode.nodeName != "assets") {
		showDialog("Connection Failure to Server", "Please Try Again");

	// something barfed server side
	} else if (assetsNode.attributes.success != "1") {
		showDialog("Server Error", assetsNode.firstChild.nodeValue);

	// everything went well, load 'em up
	} else {
		_root.list_container.loadKids(this);

	}// end if

}// end assetXMLonLoad()
