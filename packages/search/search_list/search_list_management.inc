<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_list_management.inc,v 1.5.2.1 2006/02/23 05:56:49 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_List_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5.2.1 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['default_format'] = Array(
											'added'		=> '0.1',
											'type'		=> 'wysiwyg',
											'default'	=> '%search_term% ( %number_of_occurrences% )',
										);

		$this->vars['search_field']   = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										);

	}//end constructor


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

		if (version_compare($current_version, '0.4', '<')) {
			// upgrade the search field values
			// used to be 'attribute:someName:asset_type'
			// new version is just 'someName'
			pre_echo('STARTING SEARCH LIST UPGRADE - FROM VERSION 0.3');
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				pre_echo('Upgrading Search List #'.$assetid);

				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$new_field_name = '';
				$old_name = $asset->attr('search_field');
				$pos = strpos($old_name,':');
				if ($pos !== FALSE) {
					$new_field_name = substr($old_name, $pos+1);
					$new_field_name = substr($new_field_name, 0, strpos($new_field_name,':'));
					$asset->setAttrValue('search_field', $new_field_name);
					$asset->saveAttributes();
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			pre_echo('FINISHED SEARCH PAGE UPGRADE FROM VERSION 0.3');
		}

		return TRUE;

	}//end _upgrade()


}//end class

?>