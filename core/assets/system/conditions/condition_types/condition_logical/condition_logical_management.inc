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
* $Id: condition_logical_management.inc,v 1.2 2006/12/06 04:23:58 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Condition_Logical_Management
*
* @see Asset_Management
*
* @author  Geoffroy Noel <gnoel@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Logical_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Condition_Logical_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class
?>
