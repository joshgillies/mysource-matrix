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
* $Id: user_group_management.inc,v 1.9 2006/12/05 06:04:33 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* User_Group_Management
*
* @see Asset_Management
*
* @author  Greg Serwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Group_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function User_Group_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class
?>
