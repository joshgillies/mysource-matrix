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

