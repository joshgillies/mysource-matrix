/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mcMailBoxMessageStatusClass.as,v 1.2 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

function mcMailBoxMessageStatusClass() {
}

mcMailBoxMessageStatusClass.prototype = new MovieClip();
Object.registerClass('mcMailBoxMessageStatusID', mcMailBoxMessageStatusClass);

mcMailBoxMessageStatusClass.STATUS_UNREAD	= 'U';
mcMailBoxMessageStatusClass.STATUS_READ		= 'R';


mcMailBoxMessageStatusClass.prototype.setStatus = function(status) {
	switch(status) {
		case mcMailBoxMessageStatusClass.STATUS_READ:
			this.gotoAndStop('read');
			break;

		case mcMailBoxMessageStatusClass.STATUS_UNREAD:
		default:
			this.gotoAndStop('unread');
			break;

	}
}

