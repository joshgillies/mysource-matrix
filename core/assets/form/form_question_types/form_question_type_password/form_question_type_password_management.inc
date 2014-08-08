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
* $Id: form_question_type_password_management.inc,v 1.5 2013/04/08 07:52:53 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Form_Question_Type_Password_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Type_Password_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Password_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'default'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'width'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '30',
									   ),
						'verify'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
									   ),
						'max'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '30',
									   ),
						'min'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '2',
									   ),
						'extra'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
		    				'field_name'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> 'Password',
									   ),
		    		    		'confirm_field_name'	=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> 'Confirm Password',
									   ),
					  );

	}//end constructor


}//end class
?>
