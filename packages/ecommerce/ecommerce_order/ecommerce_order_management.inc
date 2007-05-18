<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_order_management.inc,v 1.9 2007/05/18 06:05:14 colivar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Order_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Order_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Order_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'summary'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'internal_summary'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'status'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> 0,
											   ),
						'status_message'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'products'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'delivery_id'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '0',
											   ),
						'delivery_state'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'order_form_value'	=> Array(
												'added'		=> '0.2',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
					  );

	}//end constructor


	/**
	* upgrade function
	*
	* @param string	$current_version	current version of the asset
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		return $res;

	}//end _upgrade()


}//end class

?>
