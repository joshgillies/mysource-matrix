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
* $Id: form_management.inc,v 1.21 2004/06/03 04:16:30 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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

		$this->vars = Array('name'			=> Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												'update_fn' => ''
												),
									
							'wizard'		=> Array(
													'added'     => '0.0.1',
													'type'      => 'boolean', 
													'default'   => 0,
										), // end wizard
							
							'validate'		=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> 0,
										), // end validate

							'log_submissions' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> 0,
									), // end log_submission

							'show_results' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'boolean',
													'default'	=> 1,
									), // end hide_results

							'cookie_time'	=> Array(
													'added'	=> '0.0.1',
													'type'	=> 'selection',
													'default'	=> 30,
													'parameters'	=> Array(
														'options'	=> Array(
														10	=> '10 days',
														20	=> '20 days',
														30	=> '30 days',
														60	=> '60 days',
														100	=> '100 days',
														),
													),
												),
							'submit_button' => Array(
											'added'     => '0.0.1',
											'type'      => 'text',  
											'default'   => '',
								),
							'reset_button' => Array(
											'added'     => '0.0.1',
											'type'      => 'text',  
											'default'   => '',
								),
							'next_button' => Array(
											'added'     => '0.0.1',
											'type'      => 'text',  
											'default'   => '',
								),
							'back_button' => Array(
											'added'     => '0.0.1',
											'type'      => 'text',  
											'default'   => '',
								),
							'special_info' => Array(
												'added'		=> '0.0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'update_fn' => '',
											),
							'auto_answer' => Array(
												'added'		=> '0.0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'update_fn' => '',
											),

							'section_index'	=> Array(
												'added'	=> '0.0.1',
													'type'	=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(
														'options'	=> Array(
														0	=> '----none----',
														1	=> '1,2,3...',
														2	=> 'a,b,c...',
														3	=> 'A,B,C...',
														4	=> 'i,ii,iii...',
														5	=> 'I,II,III...',
															
														),
													),
									),
							'question_index'	=> Array(
												'added'	=> '0.0.1',
													'type'	=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(
														'options'	=> Array(
														0	=> '----none----',
														1	=> '1,2,3...',
														2	=> 'a,b,c...',
														3	=> 'A,B,C...',
														4	=> 'i,ii,iii...',
														5	=> 'I,II,III...',
															
														),
													),
									),
							'question_col_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'html_width',  
											'default'   => '40%',
								),
							'answer_col_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'html_width',  
											'default'   => '60%',
								),
							'form_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'html_width',  
											'default'   => '100%',
								),
							'sort_order' => Array(
											'added'     => '0.0.1',
											'type'      => 'serialise',
											'default'   => Array(),
								),
							'questions' => Array(
											'added'     => '0.0.1',
											'type'      => 'serialise',
											'default'   => Array(),
								),
							'use_bodycopy_main' => Array(
											'added'     => '0.0.1',
											'type'      => 'boolean',
											'default'   => false,
								),
							'use_bodycopy_thank_you' => Array(
											'added'     => '0.0.1',
											'type'      => 'boolean',
											'default'   => false,
								),
							'use_client_side' => Array(
											'added'     => '0.0.1',
											'type'      => 'boolean',
											'default'   => false,
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
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);
	}//end getEventList()


}//end class
?>
