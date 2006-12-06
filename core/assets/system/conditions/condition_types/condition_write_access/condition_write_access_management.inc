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
* $Id: condition_write_access_management.inc,v 1.6 2006/12/06 04:23:59 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Condition_Write_Access_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Write_Access_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Condition_Write_Access_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class
?>