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
* $Id: design_css_management.inc,v 1.6.2.1 2006/08/28 07:40:41 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.6.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Css_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['id_name'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								 );
		$this->vars['cache_css'] = Array(
									'added'			=> '0.0.1',
									'type'			=> 'boolean',
									'default'		=> FALSE,
									'parameters'	=> Array('allow_empty' => FALSE),
								   );
		$this->vars['strip_whitespace'] = Array(
											'added'		=> '0.0.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										  );

	}//end constructor


	/**
	* Upgrade this asset to the most current version
	*
	* @param string	$current_version	The version this asset is being upgraded from
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (version_compare($current_version,'0.3','<')) {
			pre_echo('UPGRADING CSS FILE ASSET TYPE - FROM VERSION '.$current_version);
			// As of v0.3 all 
			$css_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design_css');
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('design_css');
			$asset =& new Design_CSS();
			foreach ($css_assetids as $assetid) {
				$asset->load($assetid);
				$cache_id = $asset->getCacheFileId();
				if (empty($cache_id)) {
					echo 'Creating cache file for CSS File #'.$assetid."\n";
					$asset->_createCacheFile();
				}
			}
			pre_echo('DONE UPGRADING CSS FILE ASSET TYPE');
		}

		return TRUE;

	}//end _upgrade()

}//end class
?>
