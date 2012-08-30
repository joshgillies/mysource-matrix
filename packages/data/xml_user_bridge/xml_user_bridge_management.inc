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
* $Id: xml_user_bridge_management.inc,v 1.7 2012/08/30 00:58:16 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Xml_User_Bridge_Management
*
* @see Asset_Management
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $
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
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'=> TRUE,
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
