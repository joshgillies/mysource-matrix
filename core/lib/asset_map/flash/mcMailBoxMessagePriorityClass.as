/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: mcMailBoxMessagePriorityClass.as,v 1.4 2003/11/26 00:51:12 gsherwood Exp $
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
