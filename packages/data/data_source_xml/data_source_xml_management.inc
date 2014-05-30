<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_xml_management.inc,v 1.8 2012/08/30 00:58:16 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Xml_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.8 $
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
						'xslt_file'			=> Array (
												'added'			=> '0.3',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'XSL transformation file assetid',
											   ),
						'xslt_file_replace_keywords'	=> Array (
															'added'			=> '0.3',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'parameters'    => Array('allow_empty' => FALSE),
															'description'	=> 'Whether to replace keywords in the XSL file content before applying to the XML',
											   ),
												
												
											   
					  );

	}//end constructor


}//end class
?>
