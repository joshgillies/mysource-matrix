<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Page_Lineage_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Page_Lineage_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function Design_Area_Menu_Page_Lineage_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> ' : ',
								'description'	=> 'The text that divides the pages in the lineage'
							),
							'class' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'stylesheet class to use when printing the list'
							),
							'prefix_with_home_link' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to start the list with a link to the site\'s index'
							),
							'suffix_with_current_link' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to end the list with a link to the current page'
							),
							'prefix_with_divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> false,
								'description'	=> 'whether or not to start the list with a divider'
							),
							'suffix_with_divider' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> true,
								'description'	=> 'whether or not to end the list with a divider'
							),
							'have_links' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'boolean',
								'default'		=> true,
								'description'	=> 'whether or not to you want the page lineage to be hyperlinked'
							),
							'pages_to_print' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'How many pages deep to print, (zero, blank, empty means print all)'
							),
						);

	}//end constructor

}//end class
?>
