<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |

* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: paint_layout_bodycopy_management.inc,v 1.8 2006/02/09 00:41:46 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Folder_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Bodycopy_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Paint_Layout_Bodycopy_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'conditional_keywords'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'associated_asset_type'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'lineage_separator'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> ' > ',
													'description'	=> 'Seperator to use when painting the lineage of an asset.',
												   ),
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

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.4', '<')) {
			// version 0.1 -> 0.2
			// Changed Paint layouts to use their content files to read/write data that
			// can be reused. This upgrade simply prints those files for the first time
			// version 0.2 -> 0.3
			// Keywords file is always cached now at the top level, to be used with
			// content file. However this is now done with _updated() so run that instead
			// of saveSystemVersion(). This does the same thing as the 0.2 upgrade, which
			// should use the same thing now
			// version 0.3 -> 0.4
			// File ".keywords" is now "keywords" so it will properly revert.
			// SAME REMEDY AS OTHER TWO TO FIX
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.4');
			$pl_bodycopies = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			pre_echo('Updating '.count($pl_bodycopies).' asset(s)...');
			foreach ($pl_bodycopies as $assetid => $type_code) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				// We need to output our content files, otherwise old paint layouts will print blank
				$asset->_updated();

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.4');

		}//end version 0.4 upgrade

		return TRUE;

	}//end _upgrade()


}//end class
?>
