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
* $Id: data_source_xml_management.inc,v 1.2 2008/12/10 05:47:18 bshkara Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Xml_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.2 $
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
					  );

	}//end constructor


}//end class
?>
