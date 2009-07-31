<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_xml_management.inc,v 1.4 2009/07/31 05:00:52 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Xml_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage data
*/

class Data_Source_Xml_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Xml_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars	= Array (
						'name'				=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The name of the feed',
											   ),
						'url'				=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The URL of the XML feed',
											   ),
						'tags'				=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Serialized array of tag names',
											   ),
						'root_tag'			=> Array (
												'added'			=> '0.2',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Root tag for the parser',
											   ),
						'tag_attributes'	=> Array (
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Serialized array of tags and their selected attributes',
											   ),
						'dynamic_vars'		=> Array(
												'added'			=> '0.3',
												'type'			=> 'serialise',
												'default'		=> Array(),
												),
						'parameter_map'		=> Array(
												'added'			=> '0.3',
												'type'			=> 'parameter_map',
												'default'		=> Array(),
												'parameters'	=> Array(
																'parameters'	=> Array(),
															),
												),
						'cdata_tags'				=> Array (
												'added'			=> '0.3',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Serialized array of tag names which data are to be enclosed in [CDATA]',
											   ),
						'data_delimiter'			=> Array (
												'added'			=> '0.3',
												'type'			=> 'text',
												'default'		=> ',',
												'description'	=> 'Delimiter to seperate the data from xml record with mutiple elements with same name',
											   ),
											   
												
											   
					  );

	}//end constructor


}//end class
?>
