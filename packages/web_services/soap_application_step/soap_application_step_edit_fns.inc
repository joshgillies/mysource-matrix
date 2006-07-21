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
* $Id: soap_application_step_edit_fns.inc,v 1.2 2006/07/21 06:58:41 tbarrett Exp $
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
* @version $Revision: 1.2 $
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


	/**
	* Get the ID of the simple form linked to the supplied asset, if any
	*
	* @param object	&$asset	The asset under which the form may be linked
	*
	* @return int
	* @access private
	*/
	function _getFormId(&$asset)
	{
		$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'simple_form', TRUE, 'form');
		if (empty($form_link)) {
			return 0;
		} else {
			return $form_link['minorid'];
		}

	}//end _getFormId()


	/**
	* Paint the interface for adding a form
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddForm(&$asset, &$o, $prefix)
	{
		$formid = $this->_getFormId($asset);
		if (!empty($formid)) {
			echo 'This asset already has a form: '.get_asset_tag_line($formid, 'details'); // TODO: translate
		} else {
			if ($asset->writeAccess('attributes')) { // TODO: adjust lock type depending on which screen we are in
				check_box($prefix.'_add_form');
				label('Add Form', $prefix.'_add_form'); // TODO: translate
				return TRUE;
			} else {
				echo 'Acquire the lock to add a form to this asset'; // TODO: translate
			}
		}
		return FALSE;

	}//end paintAddForm()


	/**
	* Process the interface for adding a form
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddForm(&$asset, &$o, $prefix)
	{
		if (!empty($_POST[$prefix.'_add_form'])) {
			$formid = $this->_getFormId($asset);
			if (empty($formid)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('simple_form');
				$form =& new Simple_Form();
				$form->setAttrValue('name', 'Form');
				$link = Array(
							'asset'			=> &$asset,
							'link_type'		=> SQ_LINK_TYPE_2,
							'sort_order'	=> -1,
						);
				$form->create($link);
			}
		}
		return FALSE; // No need to save attrs on parent

	}//end processAddForm()


}//end class

?>