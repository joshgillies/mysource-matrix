<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Form_Section_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Section_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Form_Section_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
												'added'     => '0.0.1',
												'type'      => 'text',  
												'default'   => '',
												),
									);
	}//end constructor

}//end class
?>
