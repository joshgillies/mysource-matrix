<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_sub_asset_listing_management.inc,v 1.3 2003/10/21 06:25:44 dwong Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Sub_Asset_Listing_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Sub_Asset_Listing_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Sub_Asset_Listing_Management(&$pm)
	{
		$this->Page_Management($pm);

		$this->vars = Array(
					'name' => Array (
						'added'		=> '0.0.1',
						'type'		=> 'text',
						'default'	=> '',
					), // end name
					'types' => Array(
						'added'		=> '0.0.1',
						'type'		=> 'serialise',
						'default'	=> Array(),
					),
					'customised' => Array(
						'added'		=> '0.0.1',
						'type'		=> 'serialise',
						'default'	=> Array(),
					),
		);
	}//end constructor

	/**
	* Gets the keyword file for deployment
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'popups/keywords.php',
					 );

	}//end _getFileList()

}//end class

?>