/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: mcMailBoxMessageStatusClass.as,v 1.3 2003/11/18 15:37:35 brobertson Exp $
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

