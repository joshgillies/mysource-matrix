
/**
* This class controls certain System events, like making the System modal or not
*
*/
function SystemEvents()
{
	// holds a reference to the object that requested the modal status
	this.modal = null;

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);
}

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
	if (!(mc instanceof MovieClip)) return false;
	if (this.modal == mc)   return true;
	if (this.modal != null) return false;

	this.modal = mc;

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

	return true;

}// end stopModal()


/**
* Called to check to see if the SystemEvents is in modal status
* if it is and the passed mc is not the object that requested the modal 
* status then true is returned
*
* @param Object mc   the mc that is currently holding the modal status
*
* @return boolean
* @access public
*/
SystemEvents.prototype.inModal = function(mc) 
{
	return (this.modal != null && this.modal != mc);
}// end inModal()



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

