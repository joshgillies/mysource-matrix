<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
												'type'      => 'selection', 
												'default'   => 0,
												'parameters'	=> Array(
													'options'	=> Array(
														1	=> 'yes',
														0	=> 'no',
													),
												),
										), // end wizard
							
							'validate'		=> Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
														1	=> 'yes',
														0	=> 'no',
													),
											),
										), // end validate

							'log_submissions' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
														1	=> 'yes',
														0	=> 'no',
													),
											),
									), // end log_submission

							'hide_results' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'selection',
													'default'	=> 0,
													'parameters'	=> Array(	
														'options'	=> Array(
														1	=> 'yes',
														0	=> 'no',
													),
											),
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
							);
	}//end constructor

}//end class
?>
