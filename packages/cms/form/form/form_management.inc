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
* $Id: form_management.inc,v 1.53.2.1 2008/03/12 04:04:21 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.53.2.1 $
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
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
														'update_fn'	=> '',
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
													   ),
						'reset_button'				=> Array(
														'added'		=> '0.0.1',
														'type'		=> 'text',
														'default'	=> '',
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


}//end class
?>
