
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

  var moved = false;

  
  //document.write('<textarea id="log" rows="25" cols="100"></textarea>');
  function log(msg) {
	  //document.getElementById('log').value += msg + '\n';
  }

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
	log('starting to drag '+elt);
    movingElt = elt;
	originalParent = elt.parentNode;
	log('original parent is '+originalParent.id);
    mouseOffset = null;   
	oldCursor = elt.style.cursor;
	document.body.style.cursor='move';
	elt.style.cursor='move';
	elt.parentNode.style.cursor = 'move';
	elt.style.position = 'absolute';
    document.onmousemove = moveElt;
	document.onmouseup = stopDragging;
	log('finished starting drag');

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
	log('stopping drag');
	document.body.style.cursor='';
	movingElt.style.cursor=oldCursor;
	movingElt.parentNode.style.cursor='';
	document.onmousemove = null;
	var newCell = getDestCell(movingElt);
	var oldCell = movingElt.parentNode;
	var oldBrother = movingElt.nextSibling;
	log('New cell is '+newCell.id+'; Old cell was '+originalParent.id);
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
			log('Resetting cursor to '+oldCursor);
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
        mouseOffset = {x:(mousePosition.x - eltPos.x + parseInt(movingElt.style.marginLeft)), y:(mousePosition.y - eltPos.y + parseInt(movingElt.style.marginTop))};
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
