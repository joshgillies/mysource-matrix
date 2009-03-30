<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paypal_configuration.inc,v 1.1 2009/03/30 18:02:10 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Paypal_Configuration
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Configuration extends Asset
{


	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor

	
	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Paypal Configurations ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Paypal Configurations set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);

	}//end _createAdditional()
	

	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();
		//for better organization structure
		$allowed_links[SQ_LINK_TYPE_1]['paypal_business_account']	= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_2]['paypal_business_account']	= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_1]['paypal_payment_button']		= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_2]['paypal_payment_button']		= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_1]['paypal_ipn_receiver']		= Array('card' => 'M', 'exclusive' => FALSE);
		$allowed_links[SQ_LINK_TYPE_2]['paypal_ipn_receiver']		= Array('card' => 'M', 'exclusive' => FALSE);
		
		return $allowed_links;

	}//end _getAllowedLinks()
	
	
	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()
	
	
}//end class
?>