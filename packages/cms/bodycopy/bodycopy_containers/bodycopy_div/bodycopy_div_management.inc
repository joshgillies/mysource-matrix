<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_div_management.inc,v 1.1 2003/11/03 01:22:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_PACKAGES_PATH.'/cms/bodycopy/bodycopy_container/bodycopy_container_management.inc';

/**
* Bodycopy_Div_Management
*
* @see Bodycopy_Container_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Div_Management extends Bodycopy_Container_Management
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Div_Management(&$pm)
	{
		$this->Bodycopy_Container_Management($pm);

	}//end constructor

}//end class

?>