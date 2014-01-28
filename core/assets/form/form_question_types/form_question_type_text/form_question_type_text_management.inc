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
* $Id: form_question_type_text_management.inc,v 1.4 2012/08/30 01:09:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Text_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Text_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Text_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'default'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
									   ),
						'width'		=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '30',
									   ),
						'height'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '1',
									   ),
						'max'		=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '300',
									   ),
						'extra'		=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'text',
										'default'		=> '',
									   ),
						'required'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'DEPRECATED AS OF v0.3',
									   ),
						'text_type'	=> Array (
										'added'			=> '0.5',
										'type'			=> 'selection',
										'default'		=> 'text',
										'description'	=> 'Text to set for the "type" attribute to the input html tag',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
															'allow_other'	=> FALSE,
															'options'		=> Array(
																				'text'				=> 'Text',
																				'color'				=> 'Color',
																				'date'				=> 'Date',
																				'datetime'			=> 'Datetime',
																				'datetime-local'	=> 'Datetime-local',
																				'email'				=> 'Email',
																				'month'				=> 'Month',
																				'number'			=> 'Number',
																				'range'				=> 'Range',
																				'search'			=> 'Search',
																				'tel'				=> 'Tel',
																				'time'				=> 'Time',
																				'url'				=> 'Url',
																				'week'				=> 'Week',
																				'hidden'			=> 'Hidden',
																			   ),
														   ),
									   ),
					  );

	}//end constructor


}//end class
?>
