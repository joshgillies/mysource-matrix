<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: form_management.inc,v 1.7 2003/10/08 02:03:31 mmcintyre Exp $
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
* @subpackage __core__
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
											'type'      => 'text',  
											'default'   => '100',
								),
							'answer_col_width' => Array(
											'added'     => '0.0.1',
											'type'      => 'text',  
											'default'   => '100',
								),


							);
	}//end constructor

}//end class
?>
