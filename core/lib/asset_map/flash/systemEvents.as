/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: systemEvents.as,v 1.4 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* This class controls certain System events, like making the System modal or not
*
*/
function SystemEvents()
{
	// holds a reference to the object that requested the modal status
	this.modal      = null;
	// holds a the targetPath() of the modal object
	this.modal_path = '';

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);
}

/* Constants for use when checking checkModal() returns */
SystemEvents.NO_MODAL			= 1; // there is no modal status in effect
SystemEvents.IS_MODAL			= 2; // the passed object has acquired modal status
SystemEvents.KID_MODAL			= 4; // one of the passed objects kids has acquired modal status
SystemEvents.OTHER_MODAL		= 8; // another object has modal status

SystemEvents.NOT_OTHER_MODAL	= 7; // shorthand for NO_MODAL | IS_MODAL | KID_MODAL


/**
* Called to set the SystemEvents to modal status
*
* @param Object mc   the mc requesting modal status
*
* @return boolean
* @access public
*/
SystemEvents.prototype.startModal = function(mc) 
{
	if (this.modal !== null && this.modal != mc) return false;
	if (!(mc instanceof MovieClip)) return false;
	if (this.modal == mc)   return true;

	this.modal = mc;
	this.modal_path = targetPath(mc);

	return true;

}// end startModal()

/**
* Called to return the SystemEvents to normal status
*
* @param Object mc   the mc that is currently holding the modal status
*
* @return boolean
* @access public
*/
SystemEvents.prototype.stopModal = function(mc) 
{

	if (this.modal != mc) return false;

	this.modal = null;
	this.modal_path = '';

	return true;

}// end stopModal()


/**
* Called to check to see if the SystemEvents is in modal status
* if it is and the passed mc is not the object that requested the modal 
* status then true is returned
*
* @param Object mc   the mc that is potentially holding the modal status
*
* @return boolean
* @access public
*/
SystemEvents.prototype.inModal = function(mc) 
{
	return (this.modal !== null && this.modal !== mc);
}// end inModal()

/**
* Similar to inModal() except that it check to see if the modal status is held by 
* either the passed object or any of it's decendents
*
* @param Object mc   the mc that is potentially holding the modal status
*
* @return int
* @access public
*/
SystemEvents.prototype.checkModal = function(mc) 
{
	if (this.modal === null) return SystemEvents.NO_MODAL;
	if (this.modal === mc)   return SystemEvents.IS_MODAL;

	var path = targetPath(mc);
	if (this.modal_path.substr(0, path.length) === path) return SystemEvents.KID_MODAL;

	return SystemEvents.OTHER_MODAL;

}// end checkModal()

/**
* Returns the name of the modal child (used after a call to checkModal returns KID_MODAL)
*
* @param Object mc   the mc that is potentially holding the modal status
*
* @return string
* @access public
*/
SystemEvents.prototype.getModalChildName = function(mc) 
{
	var path = targetPath(mc);
	var i = this.modal_path.indexOf(".", path.length + 1);
	if (i < 0) i = this.modal_path.length;
	var rest = this.modal_path.substring(path.length + 1, i);
	return rest;
}// end getModalChildName()


/**
* Called whenever a press occurs on the screen
*
* @param Object mc   the mc that is currently holding the modal status
*
* @return boolean
* @access public
*/
SystemEvents.prototype.screenPress = function(mc) 
{
	this.broadcastMessage("onScreenPress", mc);
}// end screenPress()

