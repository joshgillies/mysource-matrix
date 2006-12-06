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
* $Id: system_user.inc,v 1.10 2006/12/06 00:16:07 bcaldwell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user.inc';


/**
* System_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class System_User extends Backend_User
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function System_User($assetid=0)
	{
		$this->Backend_User($assetid);

	}//end constructor


}//end class
?>
