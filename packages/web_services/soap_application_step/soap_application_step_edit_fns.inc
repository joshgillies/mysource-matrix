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
* $Id: soap_application_step_edit_fns.inc,v 1.1 2006/07/21 06:51:48 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* SOAP_Application_Step_Edit_Fns
*
* TODO: Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application_Step_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function SOAP_Application_Step_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	function paintOperationInputs(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$op_assetid = $asset->attr('soap_operation_asset');
		if (empty($op_assetid)) {
			echo "No operation asset is selected. Configuration is incomplete.";
			return $write_access;
		}

		$o->openRaw();
			echo "<h2>Input Mapper to go here. Will look like a table with 2 columns</h2>";

			// load the input map attribute
			// ask operation for inputs it accepts

			// paint input map interface
			if (!$write_access) {
				echo "<h3>You are looking at the read-only interface</h3>";
			} else {
				echo "<h3>Edit interface</h3>";
			}


		$o->closeRaw();

		return $write_access;
	}


	function processOperationInputs(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		return $write_access;
	}


}//end class

?>