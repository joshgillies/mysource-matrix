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
* $Id: template_builder_management.inc,v 1.1 2004/11/02 00:04:54 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once  SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Template_Builder_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Template_Builder_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Template_Builder_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						/// Create Locations ///
							
							'create_locations' => Array(
													'added'		=> '0.0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),					
									),
									
							);//end vars

	}//end Template_Builder_Management()


	/**
	* returns an array with information about the events that the form is listening for
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
