<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Recursive_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Recursive_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'min_num_levels' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The minimum number of levels into the hierarchy to show'
							),
							'max_num_levels' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'The maximum number of levels into the hierarchy to show, blank or zero means no maximum'
							),
						);

	}//end constructor

}//end class
?>
