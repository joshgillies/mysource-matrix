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
* $Id: design_area_head_management.inc,v 1.4 2006/12/05 22:42:13 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Head_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Head_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Head_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array();

	}//end constructor


}//end class
?>
