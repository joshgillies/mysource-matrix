/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcMailBoxMessagePriorityClass.as,v 1.2 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


function mcMailBoxMessagePriorityClass() {

}

mcMailBoxMessagePriorityClass.prototype = new MovieClip();
Object.registerClass('mcMailBoxMessagePriorityID', mcMailBoxMessagePriorityClass);

mcMailBoxMessagePriorityClass.PRIORITY_LOWEST	= 1;
mcMailBoxMessagePriorityClass.PRIORITY_LOW		= 2;
mcMailBoxMessagePriorityClass.PRIORITY_NORMAL	= 3;
mcMailBoxMessagePriorityClass.PRIORITY_HIGH		= 4;
mcMailBoxMessagePriorityClass.PRIORITY_HIGHEST	= 5;


mcMailBoxMessagePriorityClass.prototype.setPriority = function(priority) {
	priority = int(priority);
	var frame;

	switch(priority) {
		case mcMailBoxMessagePriorityClass.PRIORITY_LOWEST:
			frame = 'lowest';
			break;
		case mcMailBoxMessagePriorityClass.PRIORITY_LOW:
			frame = 'low';
			break;

		case mcMailBoxMessagePriorityClass.PRIORITY_HIGH:
			frame = 'high';
			break;

		case mcMailBoxMessagePriorityClass.PRIORITY_HIGHEST:
			frame = 'highest';
			break;

		case mcMailBoxMessagePriorityClass.PRIORITY_NORMAL:
		default:
			// if unknown type priority, set to normal
			frame = 'normal';
			break;
	}

	this.gotoAndStop(frame);

}
