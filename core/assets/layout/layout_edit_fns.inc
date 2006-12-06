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
* $Id: layout_edit_fns.inc,v 1.5 2006/12/06 01:07:34 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy_edit_fns.inc';

/**
* Layout_Edit_Fns
*
* Purpose
*
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Edit_Fns extends Bodycopy_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Layout_Edit_Fns()
	{
		$this->Bodycopy_Edit_Fns();

	}//end constructor


}//end class

?>
