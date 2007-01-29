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
* $Id: metadata_field_date_management.inc,v 1.11 2007/01/29 04:03:46 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Date_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'offset'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> ' ',
										   ),
						'format'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'd/m/Y H:i:s',
										   ),
						'setup_params'	=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'show_keywords'	=> Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> TRUE,
											'parameters'	=> Array('allow_empty' => FALSE),
										   ),
					  );

	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'popups/keywords.html',
				'js/metadata_field_date.js',
			   );

	}//end _getFileList()


}//end class

?>