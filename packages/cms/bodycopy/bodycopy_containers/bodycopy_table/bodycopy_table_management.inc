<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_table_management.inc,v 1.1 2003/11/03 01:49:49 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/bodycopy/bodycopy_container/bodycopy_container_management.inc';

/**
* Bodycopy_Table_Management
*
* @see Bodycopy_Container_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Table_Management extends Bodycopy_Container_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Table_Management(&$pm)
	{
		$this->Bodycopy_Container_Management($pm);

		$this->vars = Array('rows' => Array(
												'added'     => '0.0.1',
												'type'      => 'serialise',  'default'   => Array(),
												'update_fn' => ''
												),
							);
	}//end constructor

}//end class

?>