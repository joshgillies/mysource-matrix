<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_management.inc,v 1.8 2003/09/26 05:26:42 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  'default'   => '',
												'update_fn' => ''
												),
							'short_name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  'default'   => '',
												'update_fn' => ''
												)
							);
	}//end constructor

}//end class
?>
