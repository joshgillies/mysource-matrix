<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: data_source_record_set_management.inc,v 1.1 2005/09/28 04:09:22 pgannavarapu Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Record_Set_Management
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage core
*/
class Data_Source_Record_Set_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Record_Set_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor


}//end class

?>
