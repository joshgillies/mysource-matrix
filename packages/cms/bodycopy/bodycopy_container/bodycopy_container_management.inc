<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_container_management.inc,v 1.1 2003/11/03 01:22:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Container_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Container_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Container_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  'default'   => '',
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