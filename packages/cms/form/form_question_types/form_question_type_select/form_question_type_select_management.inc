<?php

require_once dirname(__FILE__).'/../../form_question/form_question_management.inc';

/**
* Form_Question_Type_Select_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
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
							'height' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '1',
									),
							'options' => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '',
									),
							'default' => Array(
										'added'		=> '0.0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
										'update_fn' => '',
									),
							'multiple' => Array(
									'added'		=> '0.0.1',
									'type'		=> 'selection',
									'default'	=> 0,
									'parameters'	=> Array(	
										'options'	=> Array(
											1	=> 'yes',
											0	=> 'no',
										),
								),
							'extra'	 => Array(
									'added'     => '0.0.1',
									'type'      => 'text',  
									'default'   => '',
									),
						),
								
			);
	}//end constructor

}//end class
?>
