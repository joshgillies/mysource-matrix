<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_tickbox_list_management.inc,v 1.6 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Tickbox_List_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Tickbox_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Tickbox_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'options'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'option_list',
											'default'	=> '',
										   ),
						'default'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'extra'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'required'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> false,
											'parameters'	=> Array(
																'allow_empty'	=> false,
															   ),
											'description'	=> 'DEPRECATED AS OF v0.3',
										   ),
					  );
	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.6', '<')) {
			echo('UPGRADING FORM QUESTION TYPE TICKBOX LIST FROM '.$current_version."\n");
			// Upgrade the option list
			// The option list in form_questions that use the option list (option_list, select and tickbox_list)
			// have been changed from a string delimiter by newline characters to a serialised array
			// So here we should manually upgrade questions to reflect this change

			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating Tickbox List Question Types...');

			// get all question assets
			$form_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form', FALSE, TRUE);
			$question_assets = Array();
			foreach ($form_ids as $form_id => $type) {
				$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id, $type['type_code']);
				$question_assets += $form->getChildren($form->id, 'form_question_type_tickbox_list', TRUE);
			}
			$section_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_section', FALSE, TRUE);
			foreach ($section_ids as $section_id => $type) {
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id, $type['type_code']);
				$question_assets += $section->getChildren($section->id, 'form_question_type_tickbox_list', TRUE);
			}

			// update them
			foreach ($question_assets as $id => $type) {
				$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$options = $q->attr('options');

				// Verify that the seralised options are not in array format before upgradinga
				// Don't report PHP Notice errors for the next block in case unserialize() fails
				$orig_error_reporting = error_reporting();
				error_reporting($orig_error_reporting ^ E_NOTICE);
					$options_array_format = is_array(unserialize($options));
				error_reporting($orig_error_reporting);

				if (!$options_array_format) {
					$options = str_replace("\r", '', $options);
					$opts = explode("\n", $options);
					$q->setAttrValue('options', serialize($opts));

					// Save the question
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

					if (!$q->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

						pre_echo('UPGRADE FAILED - Could not save attributes');
						return FALSE;
					}

					$count++;

					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q);
					unset($q);
				}//end if
			}//end foreach

			pre_echo('Updated Tickbox List Question Types - '.$count.' question(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo('TICKBOX LIST QUESTION TYPES SUCCESSFULLY UPGRADED TO VERSION 0.5');
		}//end if
		return TRUE;

	}//end _upgrade()


}//end class
?>
