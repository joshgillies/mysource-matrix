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
* $Id: soap_application_step_edit_fns.inc,v 1.4 2006/07/24 06:08:38 tbarrett Exp $
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
* @version $Revision: 1.4 $
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
		$formid = $asset->getFormId($asset);
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
			$asset->addForm();
		}
		return FALSE; // No need to save attrs on parent

	}//end processAddForm()


	function paintFormQuestionMappings(&$asset, &$o, $prefix)
	{
		$data_fields = $asset->getDataFields();
		if (!empty($data_fields)) {
			$o->openRaw();
			$wa = $asset->writeAccess('attributes');
			$chosen_qs = $asset->getFormQuestionMappings();
			?>
			<table class="sq-backend-table">
			<?php
			foreach ($data_fields as $codename => $details) {
				?>
				<tr>
					<th><?php echo $details['name']; ?></th>
					<td>
						<?php
						$question_id = array_get_index($chosen_qs, $codename, 0);
						if ($wa) {
							asset_finder($prefix.'_question_for_'.$codename, $question_id, Array($details['question_type'] => 'D'));
						} else if (!empty($question_id)) {
							echo get_asset_tag_line($question_id, 'details');
						}
						?>
					</td>
				</tr>
				<?php
			}
			?>
			</table>
			<?php
			$o->closeRaw();
			return TRUE;
		}
		return FALSE;

	}//end paintFormQuestionMappings()


	function processFormQuestionMappings(&$asset, &$o, $prefix)
	{
		$data_fields = $asset->getDataFields();
		foreach ($data_fields as $codename => $details) {
			if (!empty($_POST[$prefix.'_question_for_'.$codename]) && !empty($_POST[$prefix.'_question_for_'.$codename]['assetid'])) {
				$asset->setFormQuestionMapping($codename, $_POST[$prefix.'_question_for_'.$codename]['assetid']);
			}
		}
		return FALSE; // no attrs changed

	}//end processFormQuestionMappings()



	/**
	* Return whether the asset has form question mappings
	*
	* @param boolean	&$asset	The asset being edited
	*
	* @return void
	* @access public
	*/
	function hasFormQuestionMappings(&$asset)
	{
		$data_fields = $asset->getDataFields();
		return !empty($data_fields);

	}//end hasFormQuestionMappings()



}//end class

?>