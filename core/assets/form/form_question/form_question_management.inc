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
* $Id: form_question_management.inc,v 1.11 2013/04/19 06:36:00 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Question_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'note'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'rules'					=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'is_required'			=> Array(
													'added'		=> '0.3',
													'type'		=> 'boolean',
													'default'	=> FALSE,
												   ),
						'cust_required_error'	=> Array(
													'added'		=> '0.5',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'sticky'				=> Array(
													'added'			=> '0.7',
													'type'			=> 'boolean',
													'default'		=> 0,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												   ),
						'tabindex'				=> Array(
													'added'			=> '0.8',
													'type'			=> 'int',
													'default'		=> '',
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> TRUE,
																		'range_upper'		=> 32767,
																	   ),
												   ),
						'validation_condition' => Array(
													'added'		=> '0.9',
													'type'		=> 'serialise',
													'default'	=> Array(),	
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
			echo('UPGRADING FORM QUESTION FROM v < 0.6');
			// Upgrade the 'is_required' attr.  This didn't work in the 0.2 - 0.3 upgrade
			// so we need to do some trickiness here to fix failed upgrades where appropriate,
			// or do a first-time upgrade otherwise
			if (version_compare($current_version, '0.3', '<')) {
				// Simple case - just copy values from the old 'required' attrs to the new 'is_required' attr
				$fixing_upgrade = FALSE;
			} else {
				// complex case - need to preserve non-default values of the 'is_required' attr
				$fixing_upgrade = TRUE;
			}

			// get all question assets
			$form_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form', FALSE, TRUE);
			$question_assets = Array();
			foreach ($form_ids as $form_id => $type) {
				$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_id, $type);
				$question_assets += $form->getChildren($form->id, 'form_question', FALSE);
			}
			$section_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('form_section', FALSE, TRUE);
			foreach ($section_ids as $section_id => $type) {
				$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id, $type);
				$question_assets += $section->getChildren($section->id, 'form_question', FALSE);
			}

			// update them
			$irrelevant_types = Array(); // these don't have a required attr
			foreach ($question_assets as $id => $type) {
				if (!isset($irrelevant_types[$type])) {
					$q = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $type);
					if (!isset($q->vars['required'])) {
						$irrelevant_types[$type] = 1;
						continue;
					}
					if ((!$fixing_upgrade) || ($q->attr('is_required') == FALSE)) {
						// either we are doing a first-time upgrade, or we are fixing and
						// the new attr's value is still the default
						// therefore we'll copy the value from the old attr to the new
						$q->setAttrValue('is_required', $q->attr('required'));
						$q->saveAttributes();
					}
				}
			}
		}//end if
		return TRUE;

	}//end _upgrade()


}//end class
?>
