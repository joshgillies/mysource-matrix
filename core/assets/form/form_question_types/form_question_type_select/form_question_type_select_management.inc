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
* $Id: form_question_type_select_management.inc,v 1.7.8.1 2013/08/07 07:24:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

define('SQ_DELIMITER_UNIX', "\n");
define('SQ_DELIMITER_WINDOWS', "\r\n");
define('SQ_DELIMITER_MAC', "\n");

/**
* Form_Question_Type_Select_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.7.8.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Select_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Select_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'height'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '1',
										   ),
						'default'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'multiple'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'extra'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'options'		=> Array(
											'added'		=> '0.5',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'allow_empty'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'empty_key'		=> Array(
											'added'		=> '0.5',
											'type'		=> 'text',
											'default'	=> 'EMPTY',
										   ),
						'empty_text'	=> Array(
											'added'		=> '0.3',
											'type'		=> 'text',
											'default'	=> '-- Leave Empty --',
										   ),
						'allow_empty_selected'	=> Array(
											'added'			=> '0.5',
											'type'			=> 'boolean',
											'default'		=> TRUE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
		    				'empty_selected_error'	=> Array(
											'added'		=> '0.3',
											'type'		=> 'text',
											'default'	=> 'Please select a valid option.',
										   ),
					  );

	}//end constructor


	/**
	* Upgrade this asset to the current version
	*
	* @param string	$current_version	the version of the asset that is currently installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.5', '<')) {
			pre_echo('STARTING UPGRADE OF '.$asset_name.' FROM VERSION '.$current_version.' TO VERSION 0.5');

			$count = 0;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			pre_echo('Updating Select Question Types...');

			// Get a list of questions
			$form_email = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_email', FALSE, TRUE);
			$form_email += $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_section', FALSE, TRUE);
			$form_email += $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('simple_form', FALSE, TRUE);
			foreach ($form_email as $fe_id => $fe_blank) {
				$fe = $GLOBALS['SQ_SYSTEM']->am->getAsset($fe_id, $fe_blank['type_code']);

				$q_value = $fe->attr('questions');

				// Cycle thru the questions
				foreach ($q_value as $q_place_id => $q_id) {
					if (isset($q_id['type_code'])) {
						$type_code_index	= 'type_code';
					} else if (isset($q_id['question_type_code'])) {
						$type_code_index	= 'question_type_code';
					} else {
						continue;
					}//end else
					if ($q_id[$type_code_index] == 'form_question_type_select') {
						$options = $q_id['attributes']['options'];

						// Check the format stored in, and grab the options as an array
						$format_check = strpos($options, "\r");
						if ($format_check === FALSE) {
							$opt = explode(SQ_DELIMITER_UNIX, $options);
						} else {
							$opt = explode(SQ_DELIMITER_WINDOWS, $options);
						}

						// Convert the array into a format we want
						$processed_attribute = Array();
						foreach ($opt as $ol) {
							$processed_attribute[$ol] = $ol;
						}

						// Serialize the array
						$processed_attribute = serialize($processed_attribute);

						// Save back to the questions
						$q_value[$q_place_id]['attributes']['options'] = $processed_attribute;

						// Increase the count
						$count++;

					}
				}

				// Save the attribute
				$fe->setAttrValue('questions', $q_value);

				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

				if (!$fe->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

					pre_echo($asset_name.' UPGRADE FAILED - Could not save attributes');
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($fe);
				unset($fe);

			}//end foreach

			pre_echo('Updated Select Question Types - '.$count.' question(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			pre_echo($asset_name.' SUCCESSFULLY UPGRADED TO VERSION 0.5');

		}//end if

		return TRUE;

	}//end _upgrade()


}//end class
?>
