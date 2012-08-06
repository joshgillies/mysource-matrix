<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_searchbox_management.inc,v 1.16.22.1 2012/08/06 02:13:52 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_SearchBox_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.16.22.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Design_Area_SearchBox_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @access public
	*/
	function Design_Area_SearchBox_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['search_page_assetid'] = Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Search Page where queries are deferred to and results displayed on.',
											 );

		$this->vars['submit_button'] = Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'description'	=> 'Whether the search button is a button or an image.',
										'parameters'	=> Array('allow_empty' => FALSE, ),
									   );

		$this->vars['submit_button_text'] = Array(
												'added'		=> '0.2',
												'type'		=> 'text',
												'default'	=> 'Go',
											);

		$this->vars['submit_image_assetid'] = Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Image Icon Asset',
											  );

		$this->vars['querybox_width'] = Array(
											'added'			=> '0.1',
											'type'			=> 'int',
											'default'		=> '5',
											'description'	=> 'The width of the single query box.',
										);

		$this->vars['field_name'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The field defined in the search page that this is attached to.',
									);

		$this->vars['default_text'] = Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The default text to display in the search box.',
									  );

		$this->vars['category_set'] = Array(
										'added'			=> '0.3',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The category set to be used for the search box.',
									  );
		$this->vars['category_show_empty'] = Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'Should an "empty" category be listed?',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											 );
		$this->vars['category_default'] = Array(
											'added'			=> '0.3',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'Default category value',
										  );
		
		$this->vars['querybox_title'] = Array(
										'added'			=> '0.3',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The title attribute of the search input box',
									);
		$this->vars['querybox_label'] = Array(
										'added'			=> '0.3',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The label text for the search input box',
									);										  

	}//end constructor


}//end class
?>
