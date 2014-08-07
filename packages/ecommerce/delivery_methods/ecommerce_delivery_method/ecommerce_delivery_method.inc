<?php
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
* $Id: ecommerce_delivery_method.inc,v 1.15 2012/08/30 00:58:33 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Ecommerce_Delivery_Method
*
* Purpose
*	Base class for all ecommerce delivery methods
*	Delivery methods have the ability to paint/process their interface,
*	process the delivery business logic and report on the status of processing
*	The processing of interface is decoupled from execution of a delivery
*	by allowing the get/set operations on the state information
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Delivery_Method extends Asset
{

	/**
	* Container for state information
	*/
	var $state;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Ecommerce_Delivery_Method($assetid=0)
	{
		$this->state = Array();
		$this->Asset($assetid);

	}//end constructor


	/**
	* Paint Input Interface
	*
	* Produce an interface for obtaining the values required by this delivery method
	*
	* @return string	html representation of the interface
	* @access public
	*/
	function getInputInterface()
	{
		return '';

	}//end paintInputInterface()


	/**
	* Process Input
	*
	* The interface painted by paintInputInterface is processed here to populate the state
	* of this delivery method. After successfully processing the input the delivery method
	* is ready to process itself.
	*
	* @return mixed boolean|array	whether the input is valid or not
	* @access public
	*/
	function processInputInterface()
	{
		$status = $this->validateState();
		return $status;

	}//end processInputInterface()


	/**
	* Get State
	*
	* Get the complete session state of this asset. This state can later be used to pass to setState
	* in order to complete the processing. getState can be called any time, but is most effective
	* when called when the state is set - after either setState or processInput was called. If state
	* is not set will return an empty array, which is a valid state.
	*
	* @see Ecommerce_Delivery_Method::setState()
	* @return void
	* @access public
	*/
	function getState()
	{
		return $this->state;

	}//end getState()


	/**
	* Set State
	*
	* Allows this asset's state to be re-created from the state container
	* Accepts information produced by getState
	*
	* @see Ecommerce_Delivery_Method::getState()
	*
	* @param array	$state	State information
	*
	* @return void
	* @access public
	*/
	function setState($state=Array())
	{
		if (is_array($state)) {
			$this->state = $state;
		}

	}//end getState()


	/**
	* Validate State
	*
	* Determines whether the state is valid after using setState or processInput
	* Positive response means that all the input is valid and the delivery
	* is ready to be processed.
	*
	* @return boolean	whether delivery is ready to be processed
	* @access public
	*/
	function validateState()
	{
		return false;

	}//end validateState()


	/**
	* Process Delivery
	*
	* This function contains the business logic for processing the delivery
	* Positive output - delivery was processed successfully
	* Negative output - delivery Processing failed and can be retried later. Negative outcome
	* should be the result of not being able to perform the delivery, not because the input is
	* incorrect. Correctness of input should be ensured by validateState()
	*
	* @return boolean	whether delivery was processed
	* @access public
	*/
	function processDelivery()
	{
		return false;

	}//end processDelivery()


	/**
	* Get Textual Representation of delivery information (read-only)
	*
	* This information is used in receipts and order summaries
	*
	* @return string	formatted summary
	* @access public
	*/
	function getSummary()
	{
		return '';

	}//end getSummary()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Returns a receipt that can be used by the checkout to genearate order summary
	*
	* @param string	$html	set to true for HTML format, set to false for text format
	*
	* @return string
	* @access public
	*/
	function getReceipt($html=false)
	{
		return '';

	}//end getReceipt()


	/**
	* Returns a receipt summary that can be used by the checkout to be printed on the frontend
	*
	* @param string	$html	set to true for HTML format, set to false for text format
	*
	* @return string
	* @access public
	*/
	function getReceiptSummary()
	{
		return '';

	}//end getReceiptSummary()


	/**
	* Returns a confirmation summary that can be used by the checkout to be printed on the frontend
	*
	* @param string	$html	set to true for HTML format, set to false for text format
	*
	* @return string
	* @access public
	*/
	function getConfirmationSummary()
	{
		return '';

	}//end getConfirmationSummary()

	
	/**
	* Get the XML representation of the delivery method to store in the order_xml attribute of Ecommerce Order asset
	*
	* @return string
	* @access public
	*/
	function getDeliveryMethodXML()
	{
		return '';

	}//end getDeliveryMethodXML()
	

}//end class
?>
