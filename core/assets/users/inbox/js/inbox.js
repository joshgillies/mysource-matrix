var timer = null;
var read_all = true;


//shows body of current message
function showBody(msgid, td)
{
	if (document.getElementById("sq_message_body_disabled")) {
		return;
	}
	document.getElementById("sq_message_body").innerHTML = Bodies[msgid];
	clearTimeout(timer);
	timer = setTimeout("msgRead('" + msgid + "')", 1000);
	var oldrow = document.getElementById("selected_row");
	if (oldrow) {
		oldrow.id = "";
	}
	td.parentNode.parentNode.id = "selected_row";
}


//marks message as read. Calls after 1 sec after message was reded
function msgRead(msgid)
{
	var checkbox = document.getElementById(prefix + "_mark_as_read[" + msgid + "]");
	if (checkbox) {
		checkbox.checked = false;
		changeStatus(msgid, "mark_as_read", "IMG");
	}
}


//calls when user click on envelope icon
function readClick(obj)
{
	var msgid = obj.id.substring(obj.id.indexOf("[") + 1, obj.id.indexOf("]"));
	changeStatus(msgid, 'mark_as_read', obj.tagName);
}


//change status of the message
function changeStatus(msgid, readordel, tag)
{
	var checkbox1 = document.getElementById(prefix + "_mark_as_read[" + msgid + "]");
	try {
		if (tag == "IMG") {
			document.getElementById(prefix + "_" + readordel + "[" + msgid + "]").checked = !document.getElementById(prefix + "_" + readordel + "[" + msgid + "]").checked;
		}
		document.getElementById("letterIcon[" + msgid + "]").className = ((checkbox1.checked)?"":"un") + "read";
	}
	catch(e) {}

	var post = "process_form=1&am_form_submitted=1&asset_action=edit&backend_assetid=";
	post += document.getElementById("backend_assetid").value + "&";
	if (checkbox1 && checkbox1.checked) {
		post += prefix + "_mark_as_read[" + msgid + "]=on&";
	}

	post += prefix + "_messages[" + msgid + "]=on";

	var form = document.getElementById("main_form");
	JsHttpConnector.loadXMLDoc(form.action, post, form.method);
	refreshDelStatus();
}


// update top envelope icon according to other envelope icons
function refreshDelStatus()
{
	inputs = document.getElementsByTagName("INPUT");
	var allchecked = true;
	for (i = 0; i < inputs.length; i++) {
		if (inputs[i].type != "checkbox" || inputs[i].id.indexOf(prefix + "_delete[") != 0) {
			continue;
		}
		if (!inputs[i].checked) {
			allchecked = false;
			break;
		}
	}
	document.getElementById(prefix + "_delete_all").checked = allchecked;
}


//change statuses of all messages
function changeAllDelStatuses(status)
{
	var inputs = document.getElementsByTagName("INPUT");
	var post = "process_form=1&am_form_submitted=1&asset_action=edit&backend_assetid=";
	post += document.getElementById("backend_assetid").value + "&";

	for (var i = 0; i < inputs.length; i++) {
		if (inputs[i].id.indexOf("_delete[") < 0) continue;
		inputs[i].checked = status;
	}
}

