<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: bodycopy_table_management.inc,v 1.1 2003/12/02 03:31:47 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy_container/bodycopy_container_management.inc';

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