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
* $Id: authentication_default_edit_fns.inc,v 1.3 2006/12/06 04:23:57 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/authentication/authentication/authentication_edit_fns.inc';

/**
* Authentication_Default_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Authentication_Default_Edit_Fns extends Authentication_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Authentication_Default_Edit_Fns()
	{
		$this->Authentication_Edit_Fns();

	}//end constructor


}//end class

?>