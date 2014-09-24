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
* $Id: online_quiz_question_multichoice_management.inc,v 1.7 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Online_Quiz_Question_Multichoice_Management
*
* @see Asset_Management
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Online_Quiz_Question_Multichoice_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Online_Quiz_Question_Multichoice_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'response_form'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
													'description'	=> 'The options that control the response form',
												   ),
						'max_option_selections'	=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 1,
													'parameters'	=> Array(
																		'allow_negative'	=> FALSE,
																		'allow_empty'		=> FALSE,
																	   ),
													'description'	=> 'The maximum number of options a user can select when answering this question without forfeiting points',
												   ),
						'forfeit_penalty'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 0,
													'description'	=> 'The number of points a user will lose if they forfeit for whatever reason (eg. selected too many options, didn\'t answer the question)',
												   ),
						'option_style'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'selection',
													'default'		=> 'radio',
													'parameters'	=> Array(
																		'options'		=> Array(
																							'radio'	=> 'Radio Buttons / Checkboxes',
																							'list'	=> 'Dropdown List',
																						   ),
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> 'The options that control the response form',
												   ),
						'use_html_options'		=> Array(
													'added'			=> '0.3',
													'type'			=> 'selection',
													'default'		=> '0',
													'parameters'	=> Array(
																		'options'	=> Array(
																						'0'	=> 'No',
																						'1'	=> 'Yes',
																					   ),
																	   ),
													'multiple'		=> FALSE,
													'allow_empty'	=> FALSE,
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
		// Add empty Response Supplements to existing question options
		if (version_compare($current_version, '0.2', '<')) {
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			pre_echo('STARTING ONLINE QUIZ QUESTION MULTI-CHOICE UPGRADE - VERSION 0.1 => 0.2');

			foreach ($children as $assetid) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// Build the Response Supplements (option_value => response_supplement)
				$question_options = $asset->attr('response_form');

				foreach ($question_options as $option_key => $option) {
					$question_options[$option_key]['response_supplement'] = '';
				}

				if (!$asset->setAttrValue('response_form', $question_options)) {
					trigger_localised_error('CMS0039', sprintf(translate('Listing Engine Upgrade [from %s] FAILED'), '0.2'), E_USER_WARNING);
				}

				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0026', sprintf(translate('Asset Builder Upgrade [from %s] FAILED'), '0.2'), E_USER_WARNING);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			pre_echo('ONLINE QUIZ QUESTION MULTI-CHOICE UPGRADE COMPLETE - VERSION 0.1 => 0.2');
		}

		return parent::_upgrade($current_version);

	}//end _upgrade()


}//end class
?>
