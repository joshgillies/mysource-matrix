<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Submission_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Form_Submission_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Submission_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('id' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
											),
							'attributes' => Array(
												'added'		=> '0.0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'update_fn' => '',
											),
									);
	}//end constructor

}//end class
?>
