<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_asset_builder_management.inc,v 1.1 2004/01/15 05:15:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page/page_management.inc';

/**
* Page_Asset_Builder_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder_Management extends Page_Management
{

	/**
	* Constructor
	*
	*/
	function Page_Asset_Builder_Management(&$pm)
	{
		$this->Page_Management($pm);

		$this->vars['create_location']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		$this->vars['create_location_multiple']	= Array(
														'added'     => '0.1',
														'type'      => 'boolean',
														'default'   => false,
														);
		$this->vars['create_location_type']		= Array(
														'added'      => '0.1',
														'type'       => 'selection',
														'default'    => 'select',
														'parameters' => Array(
																			'options' => Array(
																					'select'	=> 'Dropdown List',
																					'check'		=> 'Checkboxes',
																			),
																		),
														);
		$this->vars['create_type']				= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '',
													);
		$this->vars['create_link_type']			= Array(
														'added'     => '0.1',
														'type'      => 'int',
														'default'   => 1,
													);
		$this->vars['create_status']			= Array(
														'added'      => '0.1',
														'type'       => 'int',
														'default'    => 2,
													);


		$this->vars['login_invite']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Sign in using the form below.</p>',
													);
		$this->vars['login_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Incorrect username or password entered. Please try again.</p>',
													);
		$this->vars['create_invite']			= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>Create an asset using the form below.</p>',
													);
		$this->vars['create_error']				= Array(
														'added'     => '0.1',
														'type'      => 'wysiwyg',
														'default'   => '<p>The new asset was not created. Ensure you have entered all required information and try again.</p>',
													);

	}//end constructor


	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'requestKeywords',
					'broadcast_type_code'		 => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options'	=> Array(
						'side_of_link'		=> 'major',
						'indirect'			=> true,
						'exclusive'			=> null,
						'dependant'			=> 1,
						'value'				=> '',
						'link_type'			=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>