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
* $Id: content_type_raw_html_management.inc,v 1.7 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_Raw_HTML_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Raw_HTML_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Raw_HTML_Management(&$pm)
	{
		$this->Asset_Management($pm);

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
				'js/raw_html_insert_keyword.js',
			   );

	}//end _getFileList()


	/**
	* Checks the preferences for Global and User Groups and updates them
	*
	* @param string	$current_version	Current version of the asset
	*
	* @return array
	* @access private
	*/
	function _upgrade($current_version)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		if (version_compare($current_version,'0.2','<')) {
			$user_group_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('user_group');
			$user_group_assets = Array();
			for ($i=0; $i < count($user_group_ids); $i++) {
				if (file_exists(SQ_DATA_PATH.'/private/assets/user_group/'.$user_group_ids[$i].'/.preferences.inc')) {
					$this->_updatePreferences(SQ_DATA_PATH.'/private/assets/user_group/'.$user_group_ids[$i].'/.preferences.inc');
				}
			}
			if (file_exists(SQ_DATA_PATH.'/private/conf/preferences.inc')) {
				$this->_updatePreferences(SQ_DATA_PATH.'/private/conf/preferences.inc');
			}
		}
		return parent::_upgrade($current_version);

	}//end _upgrade()


	/**
	* Converts the preferences to the 'ex'es and writes them into the preferences files.
	*
	* @param string	$file_name	Name of the file
	*
	* @return array
	* @access private
	*/
	function _updatePreferences($file_name)
	{
		include $file_name;
		if (!empty($preferences['content_type_raw_html']['SQ_RAW_HTML_SIZE']['default'])) {
			$width = (int) $preferences['content_type_raw_html']['SQ_RAW_HTML_SIZE']['default']['width'];
			$preferences['content_type_raw_html']['SQ_RAW_HTML_SIZE']['default']['width'] = ($width * 1.5).'ex';

			$height = (int) $preferences['content_type_raw_html']['SQ_RAW_HTML_SIZE']['default']['height'];
			$preferences['content_type_raw_html']['SQ_RAW_HTML_SIZE']['default']['height'] = ($height * 2).'ex';

			$fp = fopen($file_name, 'w');

			if ($fp) {
				fwrite($fp, '<?php $preferences = ');
				ob_start();
				var_export($preferences);
				$preferences = ob_get_contents();
				ob_end_clean();
				fwrite($fp, $preferences);
				fwrite($fp, ';?>');
				fclose($fp);
			}
		}

	}//end _updatePreferences()


}//end class

?>