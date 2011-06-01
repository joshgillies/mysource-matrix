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
* $Id: text_file.inc,v 1.5 2008/11/13 04:10:50 bpearson Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* Text_File
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Text_File extends File
{


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$parse_file = $this->data_path.'/'.$this->name;
		$content = file_to_string($parse_file);
	
		return trim($content);

	}//end getContent()


}//end class
?>
