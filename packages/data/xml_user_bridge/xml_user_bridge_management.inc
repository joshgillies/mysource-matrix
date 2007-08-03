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
* $Id: xml_user_bridge_management.inc,v 1.3 2007/08/03 00:44:29 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Xml_User_Bridge_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class XML_User_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object	&$pm	Package Manager
	*
	* @return void
	* @access public
	*/
	function XML_User_Bridge_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'data_url'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'url',
											'default'	=> '',
										   ),
						'refresh_time'	=> Array(
											'added'		=> '0.1',
											'type'		=> 'duration',
											'default'	=> 120,
										   ),
						'user_type'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'user',
										   ),
					  );


	}//end constructor


	/**
	* Gets a list of all files that the asset wants web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/user_bridge_example.xml',
			   );

	}//end _getFileList()


}//end class
?>