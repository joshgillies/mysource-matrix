<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_table_management.inc,v 1.8 2003/09/26 05:26:40 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Table_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Table_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Table_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  'default'   => '',
												'update_fn' => ''
												),
							'rows' => Array(
												'added'     => '0.0.1',
												'type'      => 'serialise',  'default'   => Array(),
												'update_fn' => ''
												),
							'attributes' => Array(
												'added'     => '0.0.1',
												'type'      => 'serialise',  'default'   => Array(),
												'update_fn' => ''
												)
							);
	}//end constructor

}//end class
?>
