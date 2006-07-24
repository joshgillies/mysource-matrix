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
* $Id: soap_application_edit_fns.inc,v 1.3 2006/07/24 23:35:04 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* SOAP_Application_Edit_Fns
*
* TODO: Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Application_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function SOAP_Application_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	function paintStepList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$o->openRaw();
		$asset->_loadSteps();
		foreach ($asset->_steps as $step_id) {
			$step =& $GLOBALS['SQ_SYSTEM']->am->getAsset($step_id);
			echo '<div>'.$step->_getName().'</div>';
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($step);
		}

		$o->closeRaw();

		return $write_access;
	}


	function processStepList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		return $write_access;
	}


	function paintAddSteps(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		echo 'will add steps hesre';
	}


	function processAddSteps(&$asset, &$o, $prefix)
	{

	}


}//end class

?>