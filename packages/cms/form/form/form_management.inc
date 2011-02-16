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
* $Id: form_management.inc,v 1.66.4.1 2011/02/16 23:32:40 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.66.4.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Form_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'						=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> '',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
														'is_contextable'=> TRUE,
													   ),
						'log_submissions'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'show_results'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'submit_button'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
														'is_contextable'	=> TRUE,
													   ),
						'reset_button'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
														'is_contextable'	=> TRUE,
													   ),
						'question_col_width'		=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'html_width',
														'default'	=> '40%',
													   ),
						'answer_col_width'			=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'html_width',
														'default'	=> '60%',
													   ),
						'form_width'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'html_width',
														'default'	=> '100%',
													   ),
						'sort_order'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'questions'					=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'serialise',
														'default'	=> Array(),
													   ),
						'use_bodycopy_main'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_bodycopy_thank_you'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_bodycopy_exit'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_bodycopy_confirmation'	=> Array(
														'added'			=> '0.13',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_bodycopy_unattached'	=> Array(
														'added'			=> '0.13',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_client_side'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'note_size'					=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '80%',
													   ),
						'next_questionid'			=> Array(
														'added'		=> '0.3',
														'type'		=> 'int',
														'default'	=> 0,
													   ),
						'submission_errors_msg'		=> Array(
														'added'		=> '0.0.2',
														'type'		=> 'wysiwyg',
														'default'	=> '',
														'is_contextable'	=> TRUE,
													   ),
						'require_captcha'			=> Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'captcha_length'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 4,
													   ),
						'captcha_zoom'				=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 1,
													   ),
						'captcha_link_text'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> 'Regenerate security key',
														'is_contextable'	=> TRUE,
													   ),
						'multi_page'				=> Array(
														'added'		=> '0.13',
														'type'		=> 'boolean',
														'default'	=> FALSE,
														'description'	=> 'If TRUE, top-level sections will be used as pages in a multi-page form, ordered by sort order. Unattached questions will appear on a first page, before the first section. If FALSE, the form will appear on a single page.',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'use_confirmation_page'		=> Array(
														'added'		=> '0.13',
														'type'		=> 'boolean',
														'default'	=> FALSE,
														'description'	=> 'Add a confirmation page after the final page of the form, allowing users of the form to review their entries before submitting.',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'next_button'			=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> 'Next Page',
														'description'	=> 'Sets the text of a Next Page button for multiple page forms. Next Page and Submit buttons both use the Submit Button keyword, however will show the Next Page text when there are still pages to view (rather than Submit on the last page).',
														'is_contextable'	=> TRUE,
													   ),
						'previous_button'		=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> 'Previous Page',
														'description'	=> 'Sets the text of a Previous Page button for multiple page forms. In a single-page form, any Previous Page button keywords will be blanked out.',
														'is_contextable'	=> TRUE,
													   ),
						'save_button'			=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> '',
														'description'	=> 'Text of the Save button for multiple-page forms. The Save button saves the current page\'s contents (without validating), and reloads the same page.',
														'is_contextable'	=> TRUE,
													   ),
						'save_confirmation_message'	=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> 'Save',
														'description'	=> 'The %save_confirmation% keyword will be replaced with this just after the Save button is pressed, otherwise keyword is blank. HTML allowed.',
														'is_contextable'	=> TRUE,
													   ),
						'save_and_exit_button'	=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> 'Save and Exit',
														'description'	=> 'Text of the Save and Exit button for multiple-page forms. Save and Exit saves the current page\'s contents (without validating), and then paints an "Exit Bodycopy". The current page counter is not changed from where the user clicked Save and Exit.',
														'is_contextable'	=> TRUE,
													   ),
						'unattached_page_name'		=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> 'Unattached Questions',
														'description'	=> 'Page name in a multi-page form for the "Unattached Questions" page (ie. questions attached directly to the form), which are displayed as Page 1 when at least one exists.',
														'is_contextable'	=> TRUE,
													   ),
						'confirmation_page_name'	=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> 'Confirmation',
														'description'	=> 'Page name in a multi-page form for the "Confirmation" page, when turned on.',
														'is_contextable'	=> TRUE,
													   ),
						'page_list_format'			=> Array(
														'added'		=> '0.13',
														'type'		=> 'text',
														'default'	=> '%page_number%',
														'description'	=> 'A format for each entry of a page list. Keywords allowed. HTML allowed, but keyword replacements will be escaped.',
													   ),
						'submission_warning_time'	=> Array(
														'added'		=> '0.13',
														'type'		=> 'duration',
														'default'	=> 0,
														'description'	=> 'Length of time before a warning email is sent out to the owner of an incomplete submission',
													   ),
						'submission_expiry_time'	=> Array(
														'added'		=> '0.13',
														'type'		=> 'duration',
														'default'	=> 0,
														'description'	=> 'Length of time until an incomplete submission is liable to be mopped up by a clean-up procedure',
													   ),
						'warning_email_format'		=> Array(
														'added'		=> '0.13',
														'type'		=> 'email_format',
														'default'	=> Array(),
													   ),
						'captcha_use_colours'		=> Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'captcha_bgcolour'			=> Array(
														'added'		=> '0.14',
														'type'		=> 'colour',
														'default'	=> 'FFFFFF',
													   ),
						'captcha_textcolour'		=> Array(
														'added'		=> '0.14',
														'type'		=> 'colour',
														'default'	=> '000000',
													   ),
						'captcha_bordercolour'		=> Array(
														'added'		=> '0.14',
														'type'		=> 'colour',
														'default'	=> '000000',
													   ),
						'submit_button_extras'		=> Array(
														'added'		=> '0.15',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'reset_button_extras'		=> Array(
														'added'		=> '0.15',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'captcha_usefont'			=> Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'captcha_ttf'				=> Array(
														'added'		=> '0.9',
														'type'		=> 'assetid',
														'default'	=> '',
														'description' => 'The asset id of the font file.',
													   ),
						'captcha_font_size'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 20,
													   ),
						'captcha_min_angle'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> -20,
													   ),
						'captcha_max_angle'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 20,
													   ),
						'captcha_x_start'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 10,
													   ),
						'captcha_min_distance'		=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 10,
													   ),
						'captcha_max_distance'		=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 20,
													   ),
						'captcha_width'				=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 200,
													   ),
						'captcha_height'			=> Array(
														'added'		=> '0.9',
														'type'		=> 'int',
														'default'	=> 36,
													   ),
						'captcha_arc'				=> Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'captcha_arc_colour'		=> Array(
														'added'		=> '0.9',
														'type'		=> 'colour',
														'default'	=> 'D20A0A',
													   ),
						'captcha_include_uppercase'	=> Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'captcha_include_numbers'	=> Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'provide_accessible_captcha'	=> Array(
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'parameters'	=> Array(
																				'allow_empty'	=> FALSE,
																			   ),
														   ),
						
						'captcha_invalid_error_message'	=> Array(
														'added'		=> '0.15',
														'type'		=> 'text',
														'default'	=> 'Security Key Incorrect',
														'is_contextable'	=> TRUE,
														   ),
						'captcha_form_invalid_error_message'	=> Array(
														'added'		=> '0.15',
														'type'		=> 'text',
														'default'	=> 'Valid CAPTCHA is required, but none is found in this form submission',
														'is_contextable'	=> TRUE,
														   ),			
			
			// Mollom Spam Check question response ///////
						'use_mollom'					=>	Array(
															'added'			=> '0.15',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'parameters'	=> Array(
																				'allow_empty'	=> FALSE,
																			),
														),
						'mollom_questions_to_check' 	=> Array(
															'added'			=> '0.15',
															'type'			=> 'serialise',
															'default'		=>  Array(),
															'description'	=> 'Question response to check for spam',
														),											
						'mollom_unsure_submission_text'	=> Array(
															'added'			=> '0.15',
															'type'			=> 'text',
															'default'		=> 'Please enter the characters in the image to verify your submission',
															'description'	=> 'Mollom error message text for unsure submission',
														),
						'mollom_spam_submission_text' 	=> Array(
															'added'			=> '0.15',
															'type'			=> 'text',
															'default'		=> 'This submission looks like a spam. Cannot accept this submission',
															'description'	=> 'Mollom error message text for spam submission',
														),
						'mollom_captcha_incorrect_text' => Array(
															'added'			=> '0.15',
															'type'			=> 'text',
															'default'		=> 'Incorrect security key. Please try again',
															'description'	=> 'Mollom error message text for incorrect captcha',
														),										   
						'mollom_captcha_regen_link_text' => Array(
															'added'			=> '0.15',
															'type'			=> 'text',
															'default'		=> 'Regenerate security key',
															'description'	=> 'Mollom CAPTCHA regen link text',
														),
						'disable_keyword_replacements'		=> Array(
															'added'			=> '0.16',
															'type'			=> 'boolean',
															'default'		=> TRUE,
															'description'	=> 'If true, will not replace any keywords from user input',
															'parameters'	=> Array(
																				'allow_empty'	=> FALSE,
													   						   ),
													  	),
					  );

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'popups/page_contents_keywords.php',
				'popups/thank_you_keywords.php',
			   );

	}//end _getFileList()


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
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.16', '<')) {
			pre_echo('STARTING FORM UPGRADE TO VERSION 0.16');

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				$asset->setAttrValue('disable_keyword_replacements', FALSE);
				if(!$asset->saveAttributes()) {
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}//end foreach

			pre_echo('FORM UPGRADE COMPLETE');

		}//end if

		return TRUE;

	}//end _upgrade()


}//end class
?>
