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
* $Id: page_user_group_manager_management.inc,v 1.4 2006/01/20 04:16:58 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Link_Manager_Management
*
* @see Page_Management
*
* @author  David Heppell <dheppell@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_User_Group_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_User_Group_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);


		$this->vars['root_node']	= Array(
										'added'			=> '0.1',
										'type'			=> 'int',
										'default'		=> '6',
										'description'	=> 'Where to start looking for locations',
									  );

		$this->vars['include_root_node']	= Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'Whether to show the top level in the selector',
												'parameters'	=> Array('allow_empty' => FALSE),
											  );

		$this->vars['link_type']	= Array(
										'added'			=> '0.1',
										'type'			=> 'selection',
										'default'		=> SQ_LINK_TYPE_1,
										'parameters'	=> Array(
															'options'	=> Array(
																			SQ_LINK_TYPE_1	=> 'Type 1',
																			SQ_LINK_TYPE_2	=> 'Type 2',
																			SQ_LINK_NOTICE	=> 'Notice',
																		   ),
														   ),
										'multiple'		=> FALSE,
										'allow_empty'	=> FALSE,
										'description'	=> 'Type of links to create for new selections',
									  );

		$this->vars['structured_width']	= Array(
											'added'			=> '0.11',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'Width of the selector',
										  );

		$this->vars['structured_height']	= Array(
												'added'			=> '0.11',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'Height of the selector',
											  );

		$this->vars['max_depth']	= Array(
										'added'			=> '0.11',
										'type'			=> 'int',
										'default'		=> 0,
										'description'	=> 'Maximum Depth of the selector',
									  );

		$this->vars['seperator']	= Array(
										'added'			=> '0.11',
										'type'			=> 'text',
										'default'		=> '-',
										'description'	=> 'Seperator for the dropdown selector',
									  );

		$this->vars['allow_multiple']	= Array(
											'added'			=> '0.11',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Should the user be able to select multiple links at once',
											'parameters'	=> Array('allow_empty' => FALSE),
										  );
		$this->vars['structured_style']	= Array(
											'added'			=> '0.12',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'Show as standard or checkbox style',
											'parameters'	=> Array('allow_empty' => FALSE),
										  );
		$this->vars['submit-button'] = Array(
										'added'			=> '0.12',
										'type'			=> 'text',
										'default'		=> 'Submit',
										'description'	=> 'Value of submit button text',
									   );

		$this->vars['statuses'] = Array(
									'added'			=> '0.8',
									'type'			=> 'serialise',
									'default'		=> Array(),
									'description'	=> 'Statuses that are being searched.',
								  );

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>
