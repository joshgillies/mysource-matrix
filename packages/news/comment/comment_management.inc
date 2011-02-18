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
* $Id: comment_management.inc,v 1.6 2011/02/18 02:18:26 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Comment_Management
*
* @see Asset_Management
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage news
*/
class Comment_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Comment_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['comment'] = Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
									'is_admin'	=> FALSE,
								 );

		$this->vars['rating'] = Array(
									'added'		=> '0.3',
									'type'		=> 'float',
									'default'	=> -1,
									'is_admin'	=> FALSE,
								);

	}//end constructor


	/**
	* Gets a list of all files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/star.png',
				'files/star_grey.png',
			   );

	}//end _getFileList()

	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.11', '<')) {			
			
			// Get the existing preferences
			include SQ_DATA_PATH.'/private/conf/preferences.inc';
			$new_preferences = $preferences;			
			
			if (!isset($new_preferences['comment']['SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS'])) {
				pre_echo('UPGRADING COMMENT GLOBAL PREFERENCES SETTINGS TO VERSION 0.11');
				
				$new_preferences['comment']['SQ_COMMENT_DISABLE_KEYWORD_REPLACEMENTS'] = Array(
																							'name' => 'comment_disable_keyword_replacements',
																							'description' => 'Whether to replace the keywords in the comment',
																							'default' => '0',
																							'protected' => FALSE,
																						);
				// Output the new preference file
				$str = '<'.'?php $preferences = '.var_export($new_preferences, TRUE).'; ?'.'>';
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				if (!string_to_file($str, SQ_DATA_PATH.'/private/conf/preferences.inc')) {
					pre_echo('COMMENT GLOBAL PREFERENCES SETTINGS UPGRADE FAILED!');
				} else {
					pre_echo('COMMENT GLOBAL PREFERENCES SETTINGS UPGRADE COMPLETED');
				}
			}//end if

		}//end if

		return TRUE;

	}//end _upgrade()

}//end class

?>
