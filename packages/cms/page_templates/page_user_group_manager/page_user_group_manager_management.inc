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
* $Id: page_user_group_manager_management.inc,v 1.8 2012/08/30 00:58:09 ewang Exp $
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
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_User_Group_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
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
										'is_contextable'	=> TRUE,
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
