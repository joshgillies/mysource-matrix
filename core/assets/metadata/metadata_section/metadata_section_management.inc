<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: metadata_section_management.inc,v 1.3 2003/09/26 05:26:27 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Section_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Section_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Metadata_Section_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
											'added'     => '0.1',
											'type'      => 'text', 'default'   => ''
											)
							);
	}//end constructor

}//end class

?>