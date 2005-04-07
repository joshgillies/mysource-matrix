<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: form_management.inc,v 1.32 2005/04/07 03:15:28 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.32 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Management extends Asset_Management
{


	/**
	* Constructor
	*
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
														'default'		=> 0,
														'parameters'	=> Array(
																			'allow_empty'	=> false,
																		   ),
													   ),
						'show_results'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> 1,
														'parameters'	=> Array(
																			'allow_empty'	=> false,
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
														'default'		=> false,
														'parameters'	=> Array(
																			'allow_empty'	=> false,
																		   ),
													   ),
						'use_bodycopy_thank_you'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> false,
														'parameters'	=> Array(
																			'allow_empty'	=> false,
																		   ),
													   ),
						'use_client_side'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> false,
														'parameters'	=> Array(
																			'allow_empty'	=> false,
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
					  );

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> false,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> true,
														'is_exclusive'	=> null,
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
	* @access private
	* @return array(string)
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
