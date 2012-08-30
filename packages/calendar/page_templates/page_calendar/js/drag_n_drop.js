/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: drag_n_drop.js,v 1.13 2012/08/30 00:57:28 ewang Exp $
*
*/

  /******************************************************
  * This script supports the dragging of elements into  *
  * or between cells of a table.                        *
  *                                                     *
  * It requires one or more draggable elements,         *
  * looking something like this:                        *
  *     <div style="position: absolute"                 *
  *          onmousedown="startDragging(this)"          *
  *          onmouseup="stopDragging(this)">            *
  * and a table to drag them into, looking something    *
  * like this:                                          *
  *     <table id="destinations">                       *
  *                                                     *
  * If you want you can define 2 optional functions     *
  *   confirmDrag(movingElt, newCell) - returns bool	*
  *   onDragFinish(movingElt, newCell)                  *
  * which will be called at the appropriate times       *
  *                                                     *
  * The script works in IE5+ and Mozilla,               *
  * possibly in other DOM browsers                      *
  *                                                     *
  * @author Tom Barrett <tbarrett@squiz.net>            *
  ******************************************************/


/* The ID of the destination table */
var destinationTableId = 'destinations';

/* The offset of the mouse pointer from the top left of the moving elt on mouse down */
var mouseOffset = null;

/* The event that's moving */
var movingElt = null;

/* The former cursor */
var oldCursor = '';

/* The element it was originally linked under */
var originalParent = null;

/* Whether the event has moved since we started dragging */
var moved = false;

/* Whether flashing is in progress (ooh) */
var flashing = '';

/**
* Start dragging the specified element
*
* @param Object   elt   The element to drag
* @return void
*/
function startDragging(elt)
{
	moved = false;
	if (movingElt !== null) return;
	while (elt.parentNode.tagName == 'DIV') {
		elt = elt.parentNode;
	}
	movingElt = elt;
	originalParent = elt.parentNode;
	mouseOffset = null;
	oldCursor = elt.style.cursor;
	document.body.style.cursor='move';
	elt.style.cursor='move';
	elt.parentNode.style.cursor = 'move';
	elt.style.position = 'absolute';
	document.onmousemove = moveElt;
	document.onmouseup = stopDragging;

}//end startDragging()


/**
* Drop the specified element into the table cell its top left corner is in, if there is one
*
* @param  Object    elt   The element that's being dragged
* @return string	  An error code representing what happened, or blank if the move completed OK
*/
function stopDragging()
{
	var result = '';
	if (movingElt == null) return 'not_dragging';
	document.body.style.cursor='';
	movingElt.style.cursor=oldCursor;
	movingElt.parentNode.style.cursor='';
	document.onmousemove = null;
	var newCell = getDestCell(movingElt);
	var oldCell = movingElt.parentNode;
	var oldBrother = movingElt.nextSibling;
	if (newCell == originalParent) {
		result = 'no_move';
		movingElt.style.left = '';
		movingElt.style.top = '';
	} else if (newCell == null) {
		result = 'left_table';
		movingElt.style.left = '';
		movingElt.style.top = '';
	} else {
		movingElt.style.left = '';
		movingElt.style.top = '';
		oldCell.removeChild(movingElt);
		if (newCell.firstChild != null) newCell.insertBefore(movingElt, newCell.firstChild);
		else newCell.appendChild(movingElt);

		if ((typeof confirmDrag != "undefined") && !confirmDrag(movingElt, newCell)) {
			result = 'user_cancelled';
			newCell.removeChild(movingElt);
			movingElt.style.cursor = oldCursor;
			if (oldBrother != null) oldCell.insertBefore(movingElt, oldBrother);
			else oldCell.appendChild(movingElt);
		} else {
			if (typeof onDragFinish != "undefined") onDragFinish(movingElt, newCell);
		}
	}
	movingElt = null;
	return result;

}//end stopDragging()


/**
* Adjust the position of the current movingElt as the mouse moves
*
* @param Object e The mouseEvent object (gecko only)
* @return void
*/
function moveElt(e)
{
	moved = true;
	var mousePosition = getMousePosition(e);
	if (mouseOffset == null) {
		// we are just starting to drag, so figure out the offset
		eltPos = getEltPosition(movingElt);
		mouseOffset = {
						x:(parseInt(mousePosition.x) - parseInt(eltPos.x)),
						y:(parseInt(mousePosition.y) - parseInt(eltPos.y))
					  };
		var ml = parseInt(movingElt.style.marginLeft)
		if (!isNaN(ml)) mouseOffset.x += ml;
		var mt = parseInt(movingElt.style.marginTop)
		if (!isNaN(mt)) mouseOffset.y += mt;
	} else {
		movingElt.style.left = (mousePosition.x - mouseOffset.x) + "px";
		movingElt.style.top = (mousePosition.y - mouseOffset.y) + "px";
	}

}//end moveElt()


/**
* Get the cell of the destination table that the specified element's top left corner is in
*
* @param  Object    elt   The element being dragged
* @return Object    The cell its top left corner is in
*/
function getDestCell(elt)
{
	var eltPos = getEltPosition(elt);
	var destTable = document.getElementById(destinationTableId);
	if (destTable == null) {
		alert('Couldn\'t find destination table '+destinationTableId);
		return null;
	}
	// get rid of moz's text nodes
	while (destTable.firstChild.nodeName == '#text') destTable.removeChild(destTable.firstChild);
	var row = destTable.firstChild.firstChild;
	while (row != null) {
		if (row.nodeName == 'TR') {
			var rowPos = getEltPosition(row);
			if ((rowPos.y <= eltPos.y) && (eltPos.y - rowPos.y < row.offsetHeight)) {
				break;
			}
		}
		row = row.nextSibling;
	}
	if (row != null) {
		var col = row.firstChild;
		while (col != null) {
			if (col.nodeName == 'TD') {
				var colPos = getEltPosition(col);
				if ((colPos.x <= eltPos.x) && (eltPos.x - colPos.x < col.offsetWidth)) {
					break;
				}
			}
			col = col.nextSibling;
		}
		if (col != null) {
			return col;
		}
	}
	return null;

}//end getDestCell()


/**
* Get the absolute co-ordinates of an element
*
* @param Object   elt   The element to find the position of
* @return Array   The co-ordinates (x=>123, y=>456)
*/
function getEltPosition(elt)
{
	var posX = 0;
	var posY = 0;
	while (elt != null) {
		posX += elt.offsetLeft;
		posY += elt.offsetTop;
		elt = elt.offsetParent;
	}
	return {x:posX,y:posY};

}//end getEltPosition()


/**
* Get the co-ordinates of the mouse pointer
*
* @param Object   e   The mousemove event (gecko only)
* @return Array   The co-ordinates (x=>123, y=>456)
*/
function getMousePosition(e)
{
	if (typeof e != "undefined") {
		// gecko
		return {x:e.pageX, y:e.pageY};
	} else if (typeof window.event != "undefined") {
		// ie
		var trueBody = (!window.opera && document.compatMode && document.compatMode!="BackCompat")? document.documentElement : document.body;
		return {x:(trueBody.scrollLeft+window.event.clientX), y:(trueBody.scrollTop+window.event.clientY)};
	}

}//end getMousePosition()


function toggleFlashing(id)
{
	elt = document.getElementById(id);
	if (flashing == elt) {
		flashing = '';
		elt.style.borderStyle = 'solid';
	} else {
		flashing = elt;
		doFlash();
	}

}//end toggleFlashing()


function doFlash()
{
	if (flashing != '') {
		flashing.style.borderStyle = (flashing.style.borderStyle.indexOf('none') != -1) ? 'dotted' : 'none';
		window.setTimeout('doFlash();', 300);
	}

}//end doFlash()


function confirmDrag(movingElt, newCell)
{
	var source_comps = originalParent.id.split('_');
	var target_comps = newCell.id.split('_');
	old_date = source_comps[1];
	old_time = source_comps[2];
	new_date = target_comps[1];
	new_time = target_comps[2];
	old_loc = (source_comps.length == 4) ?  source_comps[3] : '';
	new_loc = (target_comps.length == 4) ?  target_comps[3] : '';

	// can't drag in or out of the 'other' column
	if (Boolean(old_time == 'allday') != Boolean(new_time == 'allday')) {
		statusBarMsg(js_translate('cal_page_all_day_cannot_drag'));
		return false;
	}

	// can't drag in or out of the 'all day' row
	if (Boolean(old_loc == '*') != Boolean(new_loc == '*')) {
		statusBarMsg(js_translate('cal_page_other_cannot_drag'));
		return false;
	}
	toggleFlashing(movingElt.id);
	if (new_loc == '') {
		if (new_time == 'allday') {
			var msg = js_translate('confirm_move_event_to_date', new_date);
		} else {
			var msg = js_translate('confirm_move_event_to_datetime', new_date, new_time);
		}
	} else {
		var location_name = columnNames[String(new_loc)];
		if (new_time == 'allday') {
			var msg = js_translate('confirm_move_event_to_place_date', location_name, new_date);
		} else {
			var msg = js_translate('confirm_move_event_to_place_datetime', location_name, new_date, new_time);
		}
	}
	if (!confirm(msg)) {
		toggleFlashing(movingElt.id);
		return false;
	} else {
		return true;
	}

}//end confirmDrag()


function onDragFinish(movingElt, newCell)
{
	var source_comps = originalParent.id.split('_');
	var target_comps = newCell.id.split('_');
	if ((target_comps.length > 3) && (source_comps[3] != target_comps[3])) {
		document.getElementById('SQ_CALENDAR_OLD_LOC').value = source_comps[3];
		document.getElementById('SQ_CALENDAR_NEW_LOC').value = target_comps[3];
	}
	document.getElementById('SQ_CALENDAR_NEW_DATE').value = target_comps[1];
	if (target_comps[2] != 'allday') {
		document.getElementById('SQ_CALENDAR_NEW_TIME').value = target_comps[2];
	}
	document.getElementById('SQ_CALENDAR_EVENT_ID').value = movingElt.id.split('_')[1];
	document.getElementById('dragForm').submit();
	formSubmitted = true;

}//end onDragFinish()
